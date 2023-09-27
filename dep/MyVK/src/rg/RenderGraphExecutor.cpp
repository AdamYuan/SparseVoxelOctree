#include "RenderGraphExecutor.hpp"

#include "VkHelper.hpp"

#include <algorithm>
#include <map>
#include <span>

#ifdef MYVK_RG_DEBUG
#include <iostream>
#endif

namespace myvk_rg::_details_ {

struct RenderGraphExecutor::SubpassDependencies {
	struct SubpassDependencyKey {
		uint32_t src_subpass{}, dst_subpass{};
		VkDependencyFlags dependency_flags{};

		inline bool operator<(const SubpassDependencyKey &r) const {
			return std::tie(src_subpass, dst_subpass, dependency_flags) <
			       std::tie(r.src_subpass, r.dst_subpass, r.dependency_flags);
		}
	};
	struct AttachmentDependency {
		VkImageLayout initial_layout{VK_IMAGE_LAYOUT_UNDEFINED}, final_layout{VK_IMAGE_LAYOUT_UNDEFINED};
		bool may_alias{false};
		inline void set_initial_layout(VkImageLayout layout) {
			assert(initial_layout == VK_IMAGE_LAYOUT_UNDEFINED || initial_layout == layout);
			initial_layout = layout;
		}
		inline void set_final_layout(VkImageLayout layout) {
			assert(final_layout == VK_IMAGE_LAYOUT_UNDEFINED || final_layout == layout);
			final_layout = layout;
		}
	};
	inline void add_subpass_dependency(VkDependencyFlags dep_flags, const PassBase *src_pass,
	                                   VkPipelineStageFlags2 src_stages, VkAccessFlags src_access,
	                                   const PassBase *dst_pass, VkPipelineStageFlags2 dst_stages,
	                                   VkAccessFlags dst_access) {
		uint32_t src_subpass = src_pass && RenderGraphScheduler::GetPassID(src_pass) == pass_id
		                           ? RenderGraphScheduler::GetSubpassID(src_pass)
		                           : VK_SUBPASS_EXTERNAL;
		uint32_t dst_subpass = dst_pass && RenderGraphScheduler::GetPassID(dst_pass) == pass_id
		                           ? RenderGraphScheduler::GetSubpassID(dst_pass)
		                           : VK_SUBPASS_EXTERNAL;

		SubpassDependencyKey key{src_subpass, dst_subpass, dep_flags};

		VkMemoryBarrier2 *p_barrier;
		auto it = subpass_dependency_map.find(key);
		if (it != subpass_dependency_map.end())
			p_barrier = &it->second;
		else
			p_barrier = &subpass_dependency_map.insert({key, {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2}}).first->second;

		p_barrier->srcStageMask |= src_stages;
		p_barrier->srcAccessMask |= src_access;

		p_barrier->dstStageMask |= dst_stages;
		p_barrier->dstAccessMask |= dst_access;
	};
	inline void add_subpass_dependency(VkDependencyFlags dep_flags, const ResourceReference &link_from,
	                                   const ResourceReference &link_to, VkPipelineStageFlags2 extra_src_stages = 0,
	                                   VkAccessFlags2 extra_src_access = 0, VkPipelineStageFlags2 extra_dst_stages = 0,
	                                   VkAccessFlags2 extra_dst_access = 0) {
		add_subpass_dependency(
		    dep_flags, //
		    link_from.pass, (link_from.p_input ? link_from.p_input->GetUsagePipelineStages() : 0u) | extra_src_stages,
		    (link_from.p_input ? UsageGetWriteAccessFlags(link_from.p_input->GetUsage()) : 0u) | extra_src_access,
		    link_to.pass, (link_to.p_input ? link_to.p_input->GetUsagePipelineStages() : 0u) | extra_dst_stages,
		    (link_to.p_input ? UsageGetAccessFlags(link_to.p_input->GetUsage()) : 0u) | extra_dst_access);
	}
	inline uint32_t get_from_attachment_id(const ImageBase *image) {
		auto it = p_attachment_id_map->find(image);
		if (it != p_attachment_id_map->end())
			return it->second;
		if (image->GetClass() == ResourceClass::kLastFrameImage) {
			auto lf_image = static_cast<const LastFrameImage *>(image);
			it = p_attachment_id_map->find(lf_image->GetCurrentResource());
			if (it != p_attachment_id_map->end())
				return it->second;
		}
		it = extended_attachment_id_map.find(image);
		if (it != extended_attachment_id_map.end())
			return it->second;
		return -1;
	}
	inline uint32_t get_to_attachment_id(const ImageBase *image) {
		auto it = p_attachment_id_map->find(image);
		if (it != p_attachment_id_map->end())
			return it->second;
		it = extended_attachment_id_map.find(image);
		if (it != extended_attachment_id_map.end())
			return it->second;
		return -1;
	}

	uint32_t pass_id{};
	std::map<SubpassDependencyKey, VkMemoryBarrier2> subpass_dependency_map;
	std::vector<AttachmentDependency> attachment_dependencies;

	const std::unordered_map<const ImageBase *, uint32_t> *p_attachment_id_map{};
	std::unordered_map<const ImageBase *, uint32_t> extended_attachment_id_map;

	inline explicit SubpassDependencies(const RenderGraphScheduler::PassInfo &pass_info) {
		pass_id = RenderGraphScheduler::GetPassID(pass_info.subpasses[0].pass);
		if (pass_info.p_render_pass_info) {
			attachment_dependencies.resize(pass_info.p_render_pass_info->attachment_id_map.size());
			p_attachment_id_map = &pass_info.p_render_pass_info->attachment_id_map;
			for (const auto &origin_sub_dep : pass_info.p_render_pass_info->subpass_dependencies)
				add_subpass_dependency(VK_DEPENDENCY_BY_REGION_BIT, origin_sub_dep.from, origin_sub_dep.to);
			for (const auto &att_it : *p_attachment_id_map) {
				const auto *image = att_it.first;
				image->Visit([&att_it, this](const auto *image) {
					if constexpr (ResourceVisitorTrait<decltype(image)>::kClass == ResourceClass::kLastFrameImage) {
						auto cur_it = p_attachment_id_map->find(image->GetCurrentResource());
						if (cur_it == p_attachment_id_map->end())
							extended_attachment_id_map.insert({image->GetCurrentResource(), att_it.second});
					}
				});
			}
		}
	}
};

// Dependency Builder
struct MemoryState {
	VkPipelineStageFlags2 stage_mask;
	VkAccessFlags2 access_mask;
	VkImageLayout layout;
	bool attachment_init;

	inline bool is_valid_barrier(const MemoryState &r) const {
		return (stage_mask | access_mask) && (r.stage_mask | r.access_mask);
	}
};
class RenderGraphExecutor::DependencyBuilder {
private:
	RenderGraphExecutor &m_parent;
	std::vector<SubpassDependencies> &m_sub_deps;

	const ResourceBase *m_resource{};

	std::span<const ResourceReference> m_from_references, m_to_references;
	bool m_from_cur_frame{true}, m_to_cur_frame{true}, m_attachment_may_alias{false};

public:
	inline static MemoryState DefaultFromFunc(const ResourceReference &ref) {
		auto usage = ref.p_input->GetUsage();
		return MemoryState{ref.p_input->GetUsagePipelineStages(), UsageGetWriteAccessFlags(usage),
		                   UsageGetImageLayout(usage)};
	};
	inline static MemoryState DefaultToFunc(const ResourceReference &ref) {
		auto usage = ref.p_input->GetUsage();
		return MemoryState{ref.p_input->GetUsagePipelineStages(), UsageGetAccessFlags(usage),
		                   UsageGetImageLayout(usage)};
	};
	inline DependencyBuilder(RenderGraphExecutor *p_parent, std::vector<SubpassDependencies> *p_sub_deps,
	                         const ResourceBase *resource)
	    : m_parent{*p_parent}, m_sub_deps{*p_sub_deps}, m_resource{resource} {}
	inline void SetFromReferences(std::span<const ResourceReference> from_references, bool from_cur_frame = true) {
		m_from_references = from_references;
		m_from_cur_frame = from_cur_frame;
	}
	inline void SetToReferences(std::span<const ResourceReference> to_references, bool to_cur_frame = true) {
		m_to_references = to_references;
		m_to_cur_frame = to_cur_frame;
	}
	inline void SetAttachmentMayAlias(bool att_may_alias = true) { m_attachment_may_alias = att_may_alias; }
	template <typename FromFunc, typename ToFunc> inline void Build(FromFunc &&from_func, ToFunc &&to_func) {
		assert(m_from_cur_frame || m_to_cur_frame);

		bool in_cur_frame = m_from_cur_frame && m_to_cur_frame;

		bool is_from_attachment = m_from_references.size() == 1 && m_from_references[0].p_input &&
		                          UsageIsAttachment(m_from_references[0].p_input->GetUsage());
		bool is_to_attachment = m_to_references.size() == 1 && m_to_references[0].p_input &&
		                        UsageIsAttachment(m_to_references[0].p_input->GetUsage());

#ifdef MYVK_RG_PREFER_SUBPASS_DEPENDENCY
		// TODO: Should m_from/to_cur_frame be taken into account ?
		if (is_from_attachment || is_to_attachment) {
			// Check attachment
			assert(m_resource->GetType() == ResourceType::kImage);
			auto *image = static_cast<const ImageBase *>(m_resource);
			if (is_from_attachment &&
			    m_sub_deps[RenderGraphScheduler::GetPassID(m_from_references[0].pass)].get_from_attachment_id(image) ==
			        -1)
				is_from_attachment = false;
			if (is_to_attachment &&
			    m_sub_deps[RenderGraphScheduler::GetPassID(m_to_references[0].pass)].get_to_attachment_id(image) == -1)
				is_to_attachment = false;
		}

		if (!is_from_attachment && !is_to_attachment) {
			// Not Attachment-related, then Add a Vulkan Barrier

			// Find the optimal barrier position
			uint32_t from_bound = 0, to_bound = m_parent.m_p_scheduled->GetPassCount();
			if (m_from_cur_frame)
				for (const auto &ref : m_from_references)
					if (ref.pass)
						from_bound = std::max(from_bound, 1u + RenderGraphScheduler::GetPassID(ref.pass));
			if (m_to_cur_frame)
				for (const auto &ref : m_to_references)
					if (ref.pass)
						to_bound = std::min(to_bound, RenderGraphScheduler::GetPassID(ref.pass));

			assert(from_bound <= to_bound);

			BarrierInfo *p_barrier_info{};
			if (from_bound == 0)
				p_barrier_info = &m_parent.m_pass_executors[0].prior_barrier_info;
			else if (to_bound == m_parent.m_p_scheduled->GetPassCount())
				p_barrier_info = &m_parent.m_post_barrier_info;
			else
				p_barrier_info = &m_parent.m_pass_executors[to_bound].prior_barrier_info;
			BarrierInfo &barrier_info = *p_barrier_info;

			m_resource->Visit([this, &barrier_info, &from_func, &to_func](const auto *resource) {
				if constexpr (ResourceVisitorTrait<decltype(resource)>::kType == ResourceType::kBuffer) {
					BufferMemoryBarrier barrier = {};
					barrier.buffer = resource;

					for (const auto &ref : m_from_references) {
						MemoryState state = from_func(ref);
						barrier.src_stage_mask |= state.stage_mask;
						barrier.src_access_mask |= state.access_mask;
					}
					for (const auto &ref : m_to_references) {
						MemoryState state = to_func(ref);
						barrier.dst_stage_mask |= state.stage_mask;
						barrier.dst_access_mask |= state.access_mask;
					}
					if (barrier.is_valid_buffer_barrier())
						barrier_info.buffer_barriers.push_back(barrier);
				} else {
					ImageMemoryBarrier barrier = {};
					barrier.image = resource;

					for (const auto &ref : m_from_references) {
						MemoryState state = from_func(ref);
						barrier.src_stage_mask |= state.stage_mask;
						barrier.src_access_mask |= state.access_mask;
						assert(barrier.old_layout == VK_IMAGE_LAYOUT_UNDEFINED || barrier.old_layout == state.layout);
						barrier.old_layout = state.layout;
					}
					for (const auto &ref : m_to_references) {
						MemoryState state = to_func(ref);
						barrier.dst_stage_mask |= state.stage_mask;
						barrier.dst_access_mask |= state.access_mask;
						assert(barrier.new_layout == VK_IMAGE_LAYOUT_UNDEFINED || barrier.new_layout == state.layout);
						barrier.new_layout = state.layout;
					}
					if (barrier.is_valid_image_barrier())
						barrier_info.image_barriers.push_back(barrier);
				}
			});
		} else {
			// Add Extra Vulkan Subpass Dependencies (External) if a Dependency is Attachment-related
			assert(m_resource->GetType() == ResourceType::kImage);
			auto *image = static_cast<const ImageBase *>(m_resource);

			if (is_from_attachment && is_to_attachment) {
				const auto &ref_from = m_from_references[0], &ref_to = m_to_references[0];
				MemoryState state_from = from_func(ref_from), state_to = to_func(ref_to);
				VkImageLayout trans_layout = state_to.attachment_init ? VK_IMAGE_LAYOUT_UNDEFINED : state_from.layout;

				uint32_t from_pass_id = RenderGraphScheduler::GetPassID(ref_from.pass);
				uint32_t to_pass_id = RenderGraphScheduler::GetPassID(ref_to.pass);

				{
					auto &from_att_dep =
					    m_sub_deps[from_pass_id]
					        .attachment_dependencies[m_sub_deps[from_pass_id].get_from_attachment_id(image)];
					if (from_pass_id == to_pass_id)
						from_att_dep.may_alias |= m_attachment_may_alias;
					else if (!state_to.attachment_init)
						from_att_dep.set_final_layout(trans_layout);
				}
				{
					state_to.access_mask |= state_to.attachment_init
					                            ? VkAttachmentInitAccessFromVkFormat(image->GetFormat())
					                            : VkAttachmentLoadAccessFromVkFormat(image->GetFormat());
					state_to.stage_mask |= VkAttachmentInitialStagesFromVkFormat(image->GetFormat());

					if (state_from.is_valid_barrier(state_to))
						m_sub_deps[to_pass_id].add_subpass_dependency(
						    0,                      //
						    in_cur_frame ? ref_from.pass : nullptr, state_from.stage_mask,
						    state_from.access_mask, //
						    ref_to.pass, state_to.stage_mask, state_to.access_mask);

					auto &to_att_dep = m_sub_deps[to_pass_id]
					                       .attachment_dependencies[m_sub_deps[to_pass_id].get_to_attachment_id(image)];
					if (from_pass_id == to_pass_id)
						to_att_dep.may_alias |= m_attachment_may_alias;
					else if (!state_to.attachment_init)
						to_att_dep.set_initial_layout(trans_layout);
				}
			} else if (is_from_attachment) {
				const auto &ref_from = m_from_references[0];
				MemoryState state_from = from_func(ref_from);

				uint32_t from_pass_id = RenderGraphScheduler::GetPassID(ref_from.pass);
				auto &from_att_dep =
				    m_sub_deps[from_pass_id]
				        .attachment_dependencies[m_sub_deps[from_pass_id].get_from_attachment_id(image)];

				for (const auto &ref_to : m_to_references) {
					MemoryState state_to = to_func(ref_to);
					uint32_t to_pass_id = ref_to.pass ? RenderGraphScheduler::GetPassID(ref_to.pass) : -1;

					if (ref_to.pass && from_pass_id == to_pass_id)
						from_att_dep.may_alias |= m_attachment_may_alias;
					else if (!state_to.attachment_init)
						from_att_dep.set_final_layout(state_to.layout);

					if (state_from.is_valid_barrier(state_to))
						m_sub_deps[from_pass_id].add_subpass_dependency(0,                      //
						                                                ref_from.pass, state_from.stage_mask,
						                                                state_from.access_mask, //
						                                                in_cur_frame ? ref_to.pass : nullptr,
						                                                state_to.stage_mask, state_to.access_mask);
				}
			} else {
#ifdef MYVK_RG_DEBUG
				std::cout << "BEGIN: TO_ATT " << image->GetKey().GetName() << std::endl;
#endif
				assert(is_to_attachment);
				const auto &ref_to = m_to_references[0];
				MemoryState state_to = to_func(ref_to);

				uint32_t to_pass_id = RenderGraphScheduler::GetPassID(ref_to.pass);

				auto &to_att_dep =
				    m_sub_deps[to_pass_id].attachment_dependencies[m_sub_deps[to_pass_id].get_to_attachment_id(image)];

				state_to.access_mask |= state_to.attachment_init
				                            ? VkAttachmentInitAccessFromVkFormat(image->GetFormat())
				                            : VkAttachmentLoadAccessFromVkFormat(image->GetFormat());
				state_to.stage_mask |= VkAttachmentInitialStagesFromVkFormat(image->GetFormat());

				for (const auto &ref_from : m_from_references) {
					MemoryState state_from = from_func(ref_from);
					uint32_t from_pass_id = ref_from.pass ? RenderGraphScheduler::GetPassID(ref_from.pass) : -1;

					if (ref_from.pass && from_pass_id == to_pass_id)
						to_att_dep.may_alias |= m_attachment_may_alias;
					else if (!state_to.attachment_init) // If is init_mode, don't load
						to_att_dep.set_initial_layout(state_from.layout);

					if (state_from.is_valid_barrier(state_to)) {
						m_sub_deps[to_pass_id].add_subpass_dependency(
						    0,                      //
						    in_cur_frame ? ref_from.pass : nullptr, state_from.stage_mask,
						    state_from.access_mask, //
						    ref_to.pass, state_to.stage_mask, state_to.access_mask);
#ifdef MYVK_RG_DEBUG
						std::cout << ref_from.p_input->GetResource()->GetKey().GetName() << " " << state_from.stage_mask
						          << std::endl;
#endif
					}
				}
#ifdef MYVK_RG_DEBUG
				std::cout << "END: TO_ATT " << image->GetKey().GetName() << std::endl;
#endif
			}
		}
#else
		// TODO: is it necessary ?
		/* if (is_from_attachment || is_to_attachment) {
		    // Check attachment
		    assert(m_resource->GetType() == ResourceType::kImage);
		    auto *image = static_cast<const ImageBase *>(m_resource);
		    if (is_from_attachment &&
		        m_sub_deps[RenderGraphScheduler::GetPassID(m_from_references[0].pass)].get_from_attachment_id(image) ==
		            -1) {
		        is_from_attachment = false;
		    }
		    if (is_to_attachment &&
		        m_sub_deps[RenderGraphScheduler::GetPassID(m_to_references[0].pass)].get_to_attachment_id(image) == -1)
		{ is_to_attachment = false;
		    }
		} */

		// Process attachment inits
		if (is_to_attachment) {
			const auto &ref_to = m_to_references[0];
			MemoryState state_to = to_func(ref_to);
			auto *image = static_cast<const ImageBase *>(m_resource);

			uint32_t to_pass_id = RenderGraphScheduler::GetPassID(ref_to.pass);

			auto &to_att_dep =
			    m_sub_deps[to_pass_id].attachment_dependencies[m_sub_deps[to_pass_id].get_to_attachment_id(image)];

			state_to.access_mask |= state_to.attachment_init ? VkAttachmentInitAccessFromVkFormat(image->GetFormat())
			                                                 : VkAttachmentLoadAccessFromVkFormat(image->GetFormat());
			state_to.stage_mask |= VkAttachmentInitialStagesFromVkFormat(image->GetFormat());

			for (const auto &ref_from : m_from_references) {
				MemoryState state_from = from_func(ref_from);
				uint32_t from_pass_id = ref_from.pass ? RenderGraphScheduler::GetPassID(ref_from.pass) : -1;

				if (ref_from.pass && from_pass_id == to_pass_id)
					to_att_dep.may_alias |= m_attachment_may_alias;
				else if (!state_to.attachment_init) // If is init_mode, don't load
					to_att_dep.set_initial_layout(state_from.layout);

				if (state_from.is_valid_barrier(state_to)) {
					m_sub_deps[to_pass_id].add_subpass_dependency(
					    0,                      //
					    in_cur_frame ? ref_from.pass : nullptr, state_from.stage_mask,
					    state_from.access_mask, //
					    ref_to.pass, state_to.stage_mask, state_to.access_mask);
#ifdef MYVK_RG_DEBUG
					std::cout << ref_from.p_input->GetResource()->GetKey().GetName() << " " << state_from.stage_mask
					          << std::endl;
#endif
				}
			}
		} else {
			// Not Init-Attachment, then Add a CmdPipelineBarrier
			BarrierInfo *p_barrier_info{};
			{ // Find the optimal barrier position
				uint32_t from_bound = 0, to_bound = m_parent.m_p_scheduled->GetPassCount();
				if (m_from_cur_frame)
					for (const auto &ref : m_from_references)
						if (ref.pass)
							from_bound = std::max(from_bound, 1u + RenderGraphScheduler::GetPassID(ref.pass));
				if (m_to_cur_frame)
					for (const auto &ref : m_to_references)
						if (ref.pass)
							to_bound = std::min(to_bound, RenderGraphScheduler::GetPassID(ref.pass));
				assert(from_bound <= to_bound);
				if (from_bound == 0)
					p_barrier_info = &m_parent.m_pass_executors[0].prior_barrier_info;
				else if (to_bound == m_parent.m_p_scheduled->GetPassCount())
					p_barrier_info = &m_parent.m_post_barrier_info;
				else
					p_barrier_info = &m_parent.m_pass_executors[to_bound].prior_barrier_info;
			}
			BarrierInfo &barrier_info = *p_barrier_info;

			m_resource->Visit([this, &barrier_info, &from_func, &to_func, is_from_attachment,
			                   is_to_attachment](const auto *resource) {
				if constexpr (ResourceVisitorTrait<decltype(resource)>::kType == ResourceType::kBuffer) {
					BufferMemoryBarrier barrier = {};
					barrier.buffer = resource;

					for (const auto &ref : m_from_references) {
						MemoryState state = from_func(ref);
						barrier.src_stage_mask |= state.stage_mask;
						barrier.src_access_mask |= state.access_mask;
					}
					for (const auto &ref : m_to_references) {
						MemoryState state = to_func(ref);
						barrier.dst_stage_mask |= state.stage_mask;
						barrier.dst_access_mask |= state.access_mask;
					}
					if (barrier.is_valid_buffer_barrier())
						barrier_info.buffer_barriers.push_back(barrier);
				} else {
					ImageMemoryBarrier barrier = {};
					barrier.image = resource;

					for (const auto &ref : m_from_references) {
						MemoryState state = from_func(ref);
						barrier.src_stage_mask |= state.stage_mask;
						barrier.src_access_mask |= state.access_mask;
						assert(barrier.old_layout == VK_IMAGE_LAYOUT_UNDEFINED || barrier.old_layout == state.layout);
						barrier.old_layout = state.layout;
					}
					for (const auto &ref : m_to_references) {
						MemoryState state = to_func(ref);
						barrier.dst_stage_mask |= state.stage_mask;
						barrier.dst_access_mask |= state.access_mask;
						assert(barrier.new_layout == VK_IMAGE_LAYOUT_UNDEFINED || barrier.new_layout == state.layout);
						barrier.new_layout = state.layout;
					}
					// Add RenderPass sync info
					if (is_from_attachment) {
						const auto &ref_from = m_from_references[0];
						MemoryState state_from = from_func(ref_from);
						uint32_t from_pass_id = RenderGraphScheduler::GetPassID(ref_from.pass);
						auto &from_att_dep =
						    m_sub_deps[from_pass_id]
						        .attachment_dependencies[m_sub_deps[from_pass_id].get_from_attachment_id(resource)];

						for (const auto &ref_to : m_to_references) {
							uint32_t to_pass_id = ref_to.pass ? RenderGraphScheduler::GetPassID(ref_to.pass) : -1;

							if (ref_to.pass && from_pass_id == to_pass_id)
								from_att_dep.may_alias |= m_attachment_may_alias;
							else
								from_att_dep.set_final_layout(state_from.layout);
						}
					}
					if (is_to_attachment) {
						const auto &ref_to = m_to_references[0];
						MemoryState state_to = to_func(ref_to);
						uint32_t to_pass_id = RenderGraphScheduler::GetPassID(ref_to.pass);
						auto &to_att_dep =
						    m_sub_deps[to_pass_id]
						        .attachment_dependencies[m_sub_deps[to_pass_id].get_to_attachment_id(resource)];

						for (const auto &ref_from : m_from_references) {
							uint32_t from_pass_id = ref_from.pass ? RenderGraphScheduler::GetPassID(ref_from.pass) : -1;

							if (ref_from.pass && from_pass_id == to_pass_id)
								to_att_dep.may_alias |= m_attachment_may_alias;
							else
								to_att_dep.set_initial_layout(state_to.layout);
						}

						barrier.dst_access_mask |= VkAttachmentLoadAccessFromVkFormat(resource->GetFormat());
						barrier.dst_stage_mask |= VkAttachmentInitialStagesFromVkFormat(resource->GetFormat());
					}

					if (barrier.is_valid_image_barrier())
						barrier_info.image_barriers.push_back(barrier);
				}
			});
		}
#endif
	}
};

void RenderGraphExecutor::reset_pass_executor_vector() {
	m_pass_executors.clear();
	m_pass_executors.resize(m_p_scheduled->GetPassCount());
	for (uint32_t pass_id = 0; pass_id < m_p_scheduled->GetPassCount(); ++pass_id)
		m_pass_executors[pass_id].p_info = &m_p_scheduled->GetPassInfo(pass_id);

	m_post_barrier_info.clear();
}

void RenderGraphExecutor::reset_pass_pipeline_state() {
	for (const auto &pass_exec : m_pass_executors) {
		for (const auto &subpass : pass_exec.p_info->subpasses)
			subpass.pass->m_executor_info.pipeline_updated = true;
	}
}

void RenderGraphExecutor::_process_validation_dependency(const RenderGraphScheduler::PassDependency &dep,
                                                         std::vector<SubpassDependencies> *p_sub_deps) {
	DependencyBuilder builder{this, p_sub_deps, dep.resource};
	builder.SetToReferences(dep.to);

	std::vector<ResourceReference> mem_alias_refs{}; // if needed
	dep.resource->Visit([this, &builder, &mem_alias_refs](const auto *resource) {
		using Trait = ResourceVisitorTrait<decltype(resource)>;
		if constexpr (Trait::kIsInternal) {
			uint32_t int_res_id = m_p_resolved->GetIntResourceID(resource);
			const auto &int_res_info = m_p_resolved->GetIntResourceInfo(resource);

			for (uint32_t dep_int_res_id = 0; dep_int_res_id < m_p_resolved->GetIntResourceCount(); ++dep_int_res_id) {
				// If resource #i and current is aliased, and #i is used prior than current,
				// then make dependencies
				if (m_p_allocated->IsIntResourceAliased(dep_int_res_id, int_res_id) &&
				    m_p_resolved->IsIntResourcePrior(dep_int_res_id, int_res_id)) {
					const auto &dep_int_res_info = m_p_resolved->GetIntResourceInfo(dep_int_res_id);

					const auto dep_refs =
					    RenderGraphScheduler::GetLastReferences<Trait::kType>(dep_int_res_info.last_references);
					mem_alias_refs.insert(mem_alias_refs.end(), dep_refs.begin(), dep_refs.end());
				}
			}

			if (!mem_alias_refs.empty()) {
#ifdef MYVK_RG_DEBUG
				std::cout << "Alias Validation: " << resource->GetKey().GetName() << std::endl;
				for (const auto &i : mem_alias_refs) {
					std::cout << i.p_input->GetResource()->GetKey().GetName() << " ";
				}
				printf("\n");
#endif
				builder.SetAttachmentMayAlias(true);
				builder.SetFromReferences(mem_alias_refs, true);
			} else {
				// If used as last frame and no double buffering,
				if (m_p_resolved->GetIntResourceInfo(resource).p_last_frame_info &&
				    m_p_allocated->GetIntResourceAlloc(resource).double_buffering == false)
					builder.SetFromReferences(RenderGraphScheduler::GetLastReferences<Trait::kType>(
					                              int_res_info.p_last_frame_info->last_references),
					                          true);
				else
					builder.SetFromReferences(
					    RenderGraphScheduler::GetLastReferences<Trait::kType>(int_res_info.last_references), false);
			}
		}
	});

	builder.Build(
	    [](const ResourceReference &ref) {
		    return MemoryState{ref.p_input->GetUsagePipelineStages(), VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_UNDEFINED};
	    },
	    [](const ResourceReference &ref) {
		    auto usage = ref.p_input->GetUsage();
		    return MemoryState{ref.p_input->GetUsagePipelineStages(), UsageGetAccessFlags(usage),
		                       UsageGetImageLayout(usage), true};
	    });
}

void RenderGraphExecutor::_process_generic_dependency(const RenderGraphScheduler::PassDependency &dep,
                                                      std::vector<SubpassDependencies> *p_sub_deps) {
	DependencyBuilder builder{this, p_sub_deps, dep.resource};
	builder.SetFromReferences(dep.from);
	builder.SetToReferences(dep.to);
	builder.Build(DependencyBuilder::DefaultFromFunc, DependencyBuilder::DefaultToFunc);
}

void RenderGraphExecutor::_process_external_dependency(const RenderGraphScheduler::PassDependency &dep,
                                                       std::vector<SubpassDependencies> *p_sub_deps) {
	DependencyBuilder builder{this, p_sub_deps, dep.resource};
	bool is_initial = dep.from.front().pass == nullptr;
	ResourceReference ext_refs[1] = {{}};
	if (is_initial) {
		builder.SetFromReferences(ext_refs, false);
		builder.SetToReferences(dep.to);
		MemoryState src_state = dep.resource->Visit([](const auto *resource) -> MemoryState {
			using Trait = ResourceVisitorTrait<decltype(resource)>;
			if constexpr (Trait::kIsExternal) {
				if constexpr (Trait::kType == ResourceType::kImage)
					return {resource->GetSrcPipelineStages(), resource->GetSrcAccessFlags(), resource->GetSrcLayout()};
				else
					return {resource->GetSrcPipelineStages(), resource->GetSrcAccessFlags()};
			} else {
				assert(false);
				return {};
			}
		});
		builder.Build([src_state](const auto &) { return src_state; }, DependencyBuilder::DefaultToFunc);
	} else {
		builder.SetFromReferences(dep.from);
		builder.SetToReferences(ext_refs, true);
		MemoryState dst_state = dep.resource->Visit([](const auto *resource) -> MemoryState {
			using Trait = ResourceVisitorTrait<decltype(resource)>;
			if constexpr (Trait::kIsExternal) {
				if constexpr (Trait::kType == ResourceType::kImage)
					return {resource->GetDstPipelineStages(), resource->GetDstAccessFlags(), resource->GetDstLayout()};
				else
					return {resource->GetDstPipelineStages(), resource->GetDstAccessFlags()};
			} else {
				assert(false);
				return {};
			}
		});
		builder.Build(DependencyBuilder::DefaultFromFunc, [dst_state](const auto &) { return dst_state; });
	}
#ifdef MYVK_RG_DEBUG
	std::cout << "EXTERNAL " << dep.resource->GetKey().GetName() << (is_initial ? " INIT" : " FINAL") << std::endl;
#endif
}

void RenderGraphExecutor::_process_last_frame_dependency(const RenderGraphScheduler::PassDependency &dep,
                                                         std::vector<SubpassDependencies> *p_sub_deps) {
	// Wait for last frame's reads or writes to be done
	DependencyBuilder builder{this, p_sub_deps, dep.resource};
	builder.SetFromReferences(
	    RenderGraphScheduler::GetLastReferences(dep.resource->GetType(),
	                                            m_p_resolved->GetIntResourceInfo(dep.resource).last_references),
	    false);
	builder.SetToReferences(dep.to);
	builder.Build(DependencyBuilder::DefaultFromFunc, DependencyBuilder::DefaultToFunc);
}

std::vector<RenderGraphExecutor::SubpassDependencies> RenderGraphExecutor::extract_barriers_and_subpass_dependencies() {
	std::vector<SubpassDependencies> sub_deps;
	sub_deps.reserve(m_p_scheduled->GetPassCount());
	for (uint32_t i = 0; i < m_p_scheduled->GetPassCount(); ++i)
		sub_deps.emplace_back(m_p_scheduled->GetPassInfo(i));

	// Extract from Pass Dependencies
	for (const auto &dep : m_p_scheduled->GetPassDependencies()) {
		if (dep.type == DependencyType::kValidation)
			_process_validation_dependency(dep, &sub_deps);
		else if (dep.type == DependencyType::kDependency)
			_process_generic_dependency(dep, &sub_deps);
		else if (dep.type == DependencyType::kLastFrame)
			_process_last_frame_dependency(dep, &sub_deps);
		else if (dep.type == DependencyType::kExternal)
			_process_external_dependency(dep, &sub_deps);
	}

	return sub_deps;
}

struct SubpassDescription {
	std::vector<VkAttachmentReference2> input_attachments, color_attachments;
	std::vector<uint32_t> preserve_attachments;
	std::optional<VkAttachmentReference2> depth_attachment;
};

void RenderGraphExecutor::create_render_passes_and_framebuffers(
    std::vector<SubpassDependencies> &&subpass_dependencies) {
	for (uint32_t pass_id = 0; pass_id < m_p_scheduled->GetPassCount(); ++pass_id) {
		auto &pass_exec = m_pass_executors[pass_id];
		const auto &pass_info = *pass_exec.p_info;
		const auto &pass_sub_deps = subpass_dependencies[pass_id];

		if (pass_info.p_render_pass_info == nullptr)
			continue;

		// Subpass Dependencies
		std::vector<VkSubpassDependency2> vk_subpass_dependencies;
		vk_subpass_dependencies.reserve(pass_sub_deps.subpass_dependency_map.size());
		for (const auto &it : pass_sub_deps.subpass_dependency_map) {
			vk_subpass_dependencies.push_back({VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2});
			VkSubpassDependency2 &dep = vk_subpass_dependencies.back();

			dep.srcSubpass = it.first.src_subpass;
			dep.dstSubpass = it.first.dst_subpass;
			dep.dependencyFlags = it.first.dependency_flags;

			dep.pNext = &(it.second);
		}

		// Initialize Attachment Info
		auto &attachments = pass_exec.render_pass_info.attachments;
		attachments.resize(pass_sub_deps.attachment_dependencies.size());
		for (const auto &it : pass_info.p_render_pass_info->attachment_id_map)
			attachments[it.second].image = it.first;

		// Subpass Description (Also Update Attachment Info)
		std::vector<SubpassDescription> subpass_descriptions(pass_info.subpasses.size());
		for (uint32_t subpass_id = 0; subpass_id < pass_info.subpasses.size(); ++subpass_id) {
			const auto &subpass_info = pass_info.subpasses[subpass_id];
			auto &subpass_desc = subpass_descriptions[subpass_id];

			const auto &get_att_id = [&pass_info](const auto *image) -> uint32_t {
				if constexpr (ResourceVisitorTrait<decltype(image)>::kType == ResourceType::kImage) {
					if constexpr (ResourceVisitorTrait<decltype(image)>::kIsAlias)
						return pass_info.p_render_pass_info->attachment_id_map.at(image->GetPointedResource());
					else
						return pass_info.p_render_pass_info->attachment_id_map.at(image);
				} else {
					assert(false);
					return -1;
				}
			};

			// Input Attachments
			subpass_desc.input_attachments.reserve(subpass_info.pass->m_p_attachment_data->m_input_attachments.size());
			for (const Input *p_input : subpass_info.pass->m_p_attachment_data->m_input_attachments) {
				uint32_t att_id = p_input->GetResource()->Visit(get_att_id);
				attachments[att_id].references.push_back({p_input, subpass_id});

				subpass_desc.input_attachments.push_back({VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2});
				auto &att_ref = subpass_desc.input_attachments.back();
				att_ref.attachment = att_id;
				att_ref.layout = UsageGetImageLayout(p_input->GetUsage());
				att_ref.aspectMask = VkImageAspectFlagsFromVkFormat(
				    attachments[att_id].image->GetFormat()); // TODO: Better InputAttachment AspectMask Handling
			}

			// Color Attachments
			subpass_desc.color_attachments.reserve(subpass_info.pass->m_p_attachment_data->m_color_attachments.size());
			for (const Input *p_input : subpass_info.pass->m_p_attachment_data->m_color_attachments) {
				uint32_t att_id = p_input->GetResource()->Visit(get_att_id);
				attachments[att_id].references.push_back({p_input, subpass_id});

				subpass_desc.color_attachments.push_back({VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2});
				auto &att_ref = subpass_desc.color_attachments.back();
				att_ref.attachment = att_id;
				att_ref.layout = UsageGetImageLayout(p_input->GetUsage());
			}

			// Depth (Stencil) Attachment
			{ // TODO: Support Stencil Attachment
				const Input *p_input = subpass_info.pass->m_p_attachment_data->m_depth_attachment;
				if (p_input) {
					uint32_t att_id = p_input->GetResource()->Visit(get_att_id);
					attachments[att_id].references.push_back({p_input, subpass_id});

					subpass_desc.depth_attachment =
					    std::optional<VkAttachmentReference2>{{VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2}};
					auto &att_ref = subpass_desc.depth_attachment.value();
					att_ref.attachment = att_id;
					att_ref.layout = UsageGetImageLayout(p_input->GetUsage());
				}
			}
		}
		// Add preserve attachments
		for (uint32_t att_id = 0; att_id < attachments.size(); ++att_id) {
			const AttachmentInfo &att_info = attachments[att_id];
			for (uint32_t i = 1; i < att_info.references.size(); ++i) {
				for (uint32_t subpass_id = att_info.references[i - 1].subpass + 1;
				     subpass_id < att_info.references[i].subpass; ++subpass_id) {
					subpass_descriptions[subpass_id].preserve_attachments.push_back(att_id);
				}
			}
		}
		std::vector<VkSubpassDescription2> vk_subpass_descriptions;
		vk_subpass_descriptions.reserve(subpass_descriptions.size());
		for (auto &info : subpass_descriptions) {
			vk_subpass_descriptions.push_back({VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2});
			VkSubpassDescription2 &desc = vk_subpass_descriptions.back();

			desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

			desc.inputAttachmentCount = info.input_attachments.size();
			desc.pInputAttachments = info.input_attachments.data();

			desc.colorAttachmentCount = info.color_attachments.size();
			desc.pColorAttachments = info.color_attachments.data();
			// TODO: pResolveAttachments

			desc.preserveAttachmentCount = info.preserve_attachments.size();
			desc.pPreserveAttachments = info.preserve_attachments.data();

			desc.pDepthStencilAttachment = info.depth_attachment.has_value() ? &info.depth_attachment.value() : nullptr;
		}

		// Attachment Descriptions
		std::vector<VkAttachmentDescription2> vk_attachment_descriptions;
		vk_attachment_descriptions.reserve(pass_sub_deps.attachment_dependencies.size());
		for (uint32_t att_id = 0; att_id < attachments.size(); ++att_id) {
			const auto &att_dep = pass_sub_deps.attachment_dependencies[att_id];
			const auto &att_info = attachments[att_id];

			vk_attachment_descriptions.push_back({VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2});
			VkAttachmentDescription2 &desc = vk_attachment_descriptions.back();
			desc.flags = att_dep.may_alias ? VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT : 0u;
			desc.format = att_info.image->GetFormat();
			desc.samples = VK_SAMPLE_COUNT_1_BIT;
			desc.initialLayout = att_dep.initial_layout;
			desc.finalLayout = att_dep.final_layout;

			VkImageAspectFlags aspects = VkImageAspectFlagsFromVkFormat(desc.format);
			VkAttachmentLoadOp initial_load_op = att_info.image->Visit([](const auto *image) -> VkAttachmentLoadOp {
				if constexpr (ResourceVisitorTrait<decltype(image)>::kState == ResourceState::kExternal ||
				              ResourceVisitorTrait<decltype(image)>::kState == ResourceState::kManaged)
					return image->GetLoadOp();
				else if constexpr (ResourceVisitorTrait<decltype(image)>::kClass == ResourceClass::kLastFrameImage) {
					return VK_ATTACHMENT_LOAD_OP_LOAD;
				} else {
					assert(false);
					return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				}
			});

			desc.loadOp = desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

			VkAttachmentLoadOp load_op =
			    desc.initialLayout == VK_IMAGE_LAYOUT_UNDEFINED ? initial_load_op : VK_ATTACHMENT_LOAD_OP_LOAD;
			VkAttachmentStoreOp store_op = desc.finalLayout == VK_IMAGE_LAYOUT_UNDEFINED
			                                   ? VK_ATTACHMENT_STORE_OP_DONT_CARE
			                                   : VK_ATTACHMENT_STORE_OP_STORE;
			if (aspects & (VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT)) {
				desc.loadOp = load_op;
				desc.storeOp = store_op;
			}
			if (aspects & VK_IMAGE_ASPECT_STENCIL_BIT) {
				desc.stencilLoadOp = load_op;
				desc.stencilStoreOp = store_op;
			}

			// If Attachment is Read-only, use STORE_OP_NONE
			bool is_read_only = std::all_of(att_info.references.begin(), att_info.references.end(),
			                                [](const AttachmentInfo::AttachmentReference &ref) {
				                                return UsageIsReadOnly(ref.p_input->GetUsage());
			                                });
			if (load_op != VK_ATTACHMENT_LOAD_OP_CLEAR && is_read_only) {
				desc.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
				desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE;
			}

			// UNDEFINED finalLayout is not allowed, just use the last layout as final
			if (desc.finalLayout == VK_IMAGE_LAYOUT_UNDEFINED)
				desc.finalLayout = UsageGetImageLayout(att_info.references.back().p_input->GetUsage());
		}

		// Create RenderPass
		VkRenderPassCreateInfo2 create_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2};
		create_info.dependencyCount = vk_subpass_dependencies.size();
		create_info.pDependencies = vk_subpass_dependencies.data();

		create_info.attachmentCount = vk_attachment_descriptions.size();
		create_info.pAttachments = vk_attachment_descriptions.data();

		create_info.subpassCount = vk_subpass_descriptions.size();
		create_info.pSubpasses = vk_subpass_descriptions.data();

		pass_exec.render_pass_info.myvk_render_pass = myvk::RenderPass::Create(m_device_ptr, create_info);

#ifdef MYVK_RG_DEBUG
		printf("PASS #%u Subpass Dependency Count : %zu (subpass count = %zu) HANDLE = 0x%lx\n", pass_id,
		       vk_subpass_dependencies.size(), vk_subpass_descriptions.size(),
		       (uintptr_t)pass_exec.render_pass_info.myvk_render_pass->GetHandle());
		for (const auto i : vk_subpass_dependencies) {
			const VkMemoryBarrier2 &b = *((const VkMemoryBarrier2 *)i.pNext);
			printf("%u->%u srcStage = %lu, dstStage = %lu; srcAccess = %lu, dstAccess = %lu\n", i.srcSubpass,
			       i.dstSubpass, b.srcStageMask, b.dstStageMask, b.srcAccessMask, b.dstAccessMask);
		}
#endif

		// Create Framebuffer
		RenderGraphScheduler::RenderPassArea area = pass_info.p_render_pass_info->area;

		std::vector<VkFramebufferAttachmentImageInfo> attachment_image_infos;
		attachment_image_infos.reserve(attachments.size());
		std::vector<VkFormat> attachment_image_formats;
		attachment_image_formats.reserve(attachments.size());
		for (const auto &att_info : attachments) {
			attachment_image_infos.push_back({VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO});
			auto &image_info = attachment_image_infos.back();

			attachment_image_formats.push_back(att_info.image->GetFormat());

			image_info.usage = m_p_allocated->GetVkImage(att_info.image, false)->GetUsage();
			image_info.width = area.extent.width;
			image_info.height = area.extent.height;
			image_info.layerCount = area.layers;
			image_info.viewFormatCount = 1;
			image_info.pViewFormats = &attachment_image_formats.back();
		}
		pass_exec.render_pass_info.myvk_framebuffer = myvk::ImagelessFramebuffer::Create(
		    pass_exec.render_pass_info.myvk_render_pass, attachment_image_infos, area.extent, area.layers);
	}
}

void RenderGraphExecutor::Prepare(const myvk::Ptr<myvk::Device> &device, const RenderGraphResolver &resolved,
                                  const RenderGraphScheduler &scheduled, const RenderGraphAllocator &allocated) {
	m_device_ptr = device;
	m_p_resolved = &resolved;
	m_p_scheduled = &scheduled;
	m_p_allocated = &allocated;

	reset_pass_executor_vector();
	reset_pass_pipeline_state();
	auto subpass_dependencies = extract_barriers_and_subpass_dependencies();
	create_render_passes_and_framebuffers(std::move(subpass_dependencies));
}

void RenderGraphExecutor::CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer, bool flip) const {
	const auto cmd_pipeline_barriers = [&command_buffer, this, flip](const BarrierInfo &barrier_info) {
		if (barrier_info.empty())
			return;
		std::vector<VkBufferMemoryBarrier2> buffer_barriers;
		buffer_barriers.reserve(barrier_info.buffer_barriers.size());
		std::vector<VkImageMemoryBarrier2> image_barriers;
		image_barriers.reserve(barrier_info.image_barriers.size());

		for (const auto &info : barrier_info.buffer_barriers) {
			buffer_barriers.push_back({VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2});
			VkBufferMemoryBarrier2 &barrier = buffer_barriers.back();

			barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.srcAccessMask = info.src_access_mask;
			barrier.dstAccessMask = info.dst_access_mask;
			barrier.srcStageMask = info.src_stage_mask;
			barrier.dstStageMask = info.dst_stage_mask;

			const myvk::Ptr<myvk::BufferBase> &myvk_buffer = m_p_allocated->GetVkBuffer(info.buffer, flip);
			barrier.buffer = myvk_buffer->GetHandle();
			barrier.size = myvk_buffer->GetSize();
			barrier.offset = 0u;
		}

		for (const auto &info : barrier_info.image_barriers) {
			image_barriers.push_back({VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2});
			VkImageMemoryBarrier2 &barrier = image_barriers.back();

			barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.oldLayout = info.old_layout;
			barrier.newLayout = info.new_layout;
			barrier.srcAccessMask = info.src_access_mask;
			barrier.dstAccessMask = info.dst_access_mask;
			barrier.srcStageMask = info.src_stage_mask;
			barrier.dstStageMask = info.dst_stage_mask;

			const myvk::Ptr<myvk::ImageView> &myvk_image_view = m_p_allocated->GetVkImageView(info.image, flip);
			barrier.image = myvk_image_view->GetImagePtr()->GetHandle();
			barrier.subresourceRange = myvk_image_view->GetSubresourceRange();
		}

		VkDependencyInfo dep_info = {VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
		dep_info.bufferMemoryBarrierCount = buffer_barriers.size();
		dep_info.pBufferMemoryBarriers = buffer_barriers.data();
		dep_info.imageMemoryBarrierCount = image_barriers.size();
		dep_info.pImageMemoryBarriers = image_barriers.data();

		vkCmdPipelineBarrier2(command_buffer->GetHandle(), &dep_info);
	};

	const auto execute_pass = [&command_buffer](const PassBase *pass) {
		if (pass->m_executor_info.pipeline_updated) {
			((PassBase *)pass)->CreatePipeline();
			pass->m_executor_info.pipeline_updated = false;
		}
		pass->CmdExecute(command_buffer);
	};

	for (const auto &pass_exec : m_pass_executors) {
		cmd_pipeline_barriers(pass_exec.prior_barrier_info);
		const auto &pass_info = *pass_exec.p_info;
		if (pass_info.p_render_pass_info) {
			const auto &attachment_infos = pass_exec.render_pass_info.attachments;
			// Fetch Attachment Clear Values and Attachment Image Views
			std::vector<VkClearValue> clear_values;
			std::vector<VkImageView> attachment_image_views;
			clear_values.reserve(attachment_infos.size());
			attachment_image_views.reserve(attachment_infos.size());

			for (const auto &att_info : attachment_infos) {
				att_info.image->Visit([this, flip, &clear_values, &attachment_image_views](const auto *image) {
					if constexpr (ResourceVisitorTrait<decltype(image)>::kClass == ResourceClass::kManagedImage ||
					              ResourceVisitorTrait<decltype(image)>::kClass == ResourceClass::kExternalImageBase) {
						clear_values.push_back(image->GetClearValue());
						attachment_image_views.push_back(m_p_allocated->GetVkImageView(image, flip)->GetHandle());
					} else if constexpr (ResourceVisitorTrait<decltype(image)>::kClass ==
					                     ResourceClass::kLastFrameImage) {
						clear_values.emplace_back();
						attachment_image_views.push_back(
						    m_p_allocated->GetVkImageView(image, flip)->GetHandle()); // TODO: just use image
					} else {
						assert(false);
					}
				});
			}

			VkRenderPassAttachmentBeginInfo attachment_begin_info = {
			    VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO};
			attachment_begin_info.attachmentCount = attachment_image_views.size();
			attachment_begin_info.pAttachments = attachment_image_views.data();

			VkRenderPassBeginInfo render_begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
			render_begin_info.renderPass = pass_exec.render_pass_info.myvk_render_pass->GetHandle();
			render_begin_info.framebuffer = pass_exec.render_pass_info.myvk_framebuffer->GetHandle();
			render_begin_info.renderArea.offset = {0u, 0u};
			render_begin_info.renderArea.extent = pass_info.p_render_pass_info->area.extent;
			render_begin_info.clearValueCount = clear_values.size();
			render_begin_info.pClearValues = clear_values.data();
			render_begin_info.pNext = &attachment_begin_info;

			vkCmdBeginRenderPass(command_buffer->GetHandle(), &render_begin_info, VK_SUBPASS_CONTENTS_INLINE);

			execute_pass(pass_info.subpasses.front().pass);
			for (uint32_t i = 1; i < pass_info.subpasses.size(); ++i) {
				vkCmdNextSubpass(command_buffer->GetHandle(), VK_SUBPASS_CONTENTS_INLINE);
				execute_pass(pass_info.subpasses[i].pass);
			}

			vkCmdEndRenderPass(command_buffer->GetHandle());
		} else {
			assert(pass_info.subpasses.size() == 1);
			execute_pass(pass_info.subpasses.front().pass);
		}
	}
	cmd_pipeline_barriers(m_post_barrier_info);
}

} // namespace myvk_rg::_details_

#include "RenderGraphScheduler.hpp"

#include <algorithm>

#ifdef MYVK_RG_DEBUG
#include <iostream>
#endif

namespace myvk_rg::_details_ {

struct RenderGraphScheduler::RenderPassMergeInfo {
	RenderPassArea area{};
	uint32_t merge_length{};
};

struct ExtraEdgeInfo {
	std::vector<RenderGraphResolver::PassEdge> input_edges;
};

std::vector<RenderGraphScheduler::RenderPassMergeInfo>
RenderGraphScheduler::_compute_pass_merge_info(const RenderGraphResolver &resolved) {
	const uint32_t kOrderedPassCount = resolved.GetPassNodeCount();

	// Add Extra Graph Edges to test Unable-to-merge Image Reads
	std::vector<ExtraEdgeInfo> extra_edge_infos(kOrderedPassCount);
	{
		const auto add_extra_edges = [&extra_edge_infos](
		                                 const std::vector<const RenderGraphResolver::PassEdge *> &output_edges) {
			std::unordered_map<const ResourceBase *, const RenderGraphResolver::PassEdge *> res_prev_edge;
			for (auto *p_edge : output_edges) {
				if (p_edge->resource->GetType() != ResourceType::kImage || p_edge->to.pass == nullptr)
					continue;
				auto it = res_prev_edge.find(p_edge->resource);
				if (it != res_prev_edge.end()) {
					uint32_t to_order = RenderGraphResolver::GetPassOrder(p_edge->to.pass);
					extra_edge_infos[to_order].input_edges.push_back({p_edge->resource, it->second->to, p_edge->to});
					it->second = p_edge;
				} else
					res_prev_edge[p_edge->resource] = p_edge;
			}
		};
		add_extra_edges(resolved.GetSrcOutputEdges());
		for (auto &node : resolved.GetPassNodes())
			add_extra_edges(node.output_edges);
	}

	std::vector<RenderPassMergeInfo> merge_infos(kOrderedPassCount);

	// Compute RenderPassArea
	for (uint32_t i = 0; i < kOrderedPassCount; ++i) {
		const PassBase *pass = resolved.GetPassNode(i).pass;
		if (pass->m_p_attachment_data == nullptr)
			continue;

		RenderPassArea &area = merge_infos[i].area;
		const auto maintain_area = [&area](const auto *resource) {
			if constexpr (ResourceVisitorTrait<decltype(resource)>::kType == ResourceType::kImage) {
				RenderPassArea desired_area;
				if constexpr (ResourceVisitorTrait<decltype(resource)>::kClass == ResourceClass::kManagedImage) {
					const SubImageSize &size = resource->GetSize();
					desired_area = {{std::max(1u, size.GetExtent().width >> size.GetBaseMipLevel()),
					                 std::max(1u, size.GetExtent().height >> size.GetBaseMipLevel())},
					                size.GetArrayLayers()};
				} else if constexpr (ResourceVisitorTrait<decltype(resource)>::kClass ==
				                     ResourceClass::kExternalImageBase) {
					VkExtent2D extent = resource->GetVkImageView()->GetImagePtr()->GetExtent2D();
					const VkImageSubresourceRange &range = resource->GetVkImageView()->GetSubresourceRange();
					desired_area = {{std::max(1u, extent.width >> range.baseMipLevel),
					                 std::max(1u, extent.height >> range.baseMipLevel)},
					                range.layerCount};
				} else if constexpr (ResourceVisitorTrait<decltype(resource)>::kClass ==
				                     ResourceClass::kLastFrameImage) {
					resource->GetCurrentResource()->Visit([&desired_area](const auto *image) {
						if constexpr (ResourceVisitorTrait<decltype(image)>::kClass == ResourceClass::kManagedImage) {
							const SubImageSize &size = image->GetSize();
							desired_area = {{std::max(1u, size.GetExtent().width >> size.GetBaseMipLevel()),
							                 std::max(1u, size.GetExtent().height >> size.GetBaseMipLevel())},
							                size.GetArrayLayers()};
						} else
							assert(false);
					});
				} else
					assert(false);
				if (area.layers == 0)
					area = desired_area;
				else
					assert(area == desired_area);
			} else
				assert(false);
		};
		pass->for_each_input([&maintain_area](const Input *p_input) {
			if (!UsageIsAttachment(p_input->GetUsage()))
				return;
			p_input->GetResource()->Visit([&maintain_area](const auto *resource) {
				if constexpr (ResourceVisitorTrait<decltype(resource)>::kIsAlias) {
					resource->GetPointedResource()->Visit(maintain_area);
				} else
					maintain_area(resource);
			});
		});
	}

	// Compute Merge Length

	// Calculate merge_length, Complexity: O(N + M)
	// merge_length == 0: The pass is not a graphics pass
	// merge_length == 1: The pass is a graphics pass, but can't be merged
	// merge_length >  1: The pass is a graphics pass, and it can be merged to a group of _merge_length_ with the
	// passes before
	{
		merge_infos[0].merge_length = resolved.GetPassNode((uint32_t)0).pass->m_p_attachment_data ? 1u : 0u;
		for (uint32_t i = 1; i < kOrderedPassCount; ++i) {
			if (resolved.GetPassNode(i).pass->m_p_attachment_data) {
				// Both are RenderPass and have equal RenderArea
				merge_infos[i].merge_length =
				    merge_infos[i - 1].area == merge_infos[i].area ? merge_infos[i - 1].merge_length + 1 : 1;
			} else
				merge_infos[i].merge_length = 0;
		}
	}
	for (uint32_t i = 0; i < kOrderedPassCount; ++i) {
		auto &length = merge_infos[i].merge_length;
		if (length <= 1)
			continue;
		const auto test_edge = [&length, &merge_infos, i](const RenderGraphResolver::PassEdge *p_edge) {
			if (!p_edge->from.pass)
				return;
			if (!UsageIsAttachment(p_edge->to.p_input->GetUsage()) ||
			    (p_edge->from.p_input && !UsageIsAttachment(p_edge->from.p_input->GetUsage()))) {
				// If an input dependency is not attachment, then all its producers can't be merged
				// Or an input dependency is attachment, but it is not produced as an attachment, then the producer
				// can't be merged
				length = std::min(length, i - RenderGraphResolver::GetPassOrder(p_edge->from.pass));
			} else if (p_edge->from.p_input) {
				// If the input dependencies are both attachments
				assert(p_edge->from.pass);
				uint32_t from_order = RenderGraphResolver::GetPassOrder(p_edge->from.pass);
				length = std::min(length, i - from_order + merge_infos[from_order].merge_length);
			}
		};
		const auto test_extra_edge = [&length, &merge_infos, i](const RenderGraphResolver::PassEdge *p_edge) {
			if (!p_edge->from.pass)
				return;
			if (UsageIsAttachment(p_edge->to.p_input->GetUsage()) !=
			    UsageIsAttachment(p_edge->from.p_input->GetUsage())) {
				// If an input dependency is not attachment, then all its producers can't be merged
				// Or an input dependency is attachment, but it is not produced as an attachment, then the producer
				// can't be merged
				length = std::min(length, i - RenderGraphResolver::GetPassOrder(p_edge->from.pass));
			} else if (p_edge->from.p_input) {
				// If the input dependencies are both attachments
				assert(p_edge->from.pass);
				uint32_t from_order = RenderGraphResolver::GetPassOrder(p_edge->from.pass);
				length = std::min(length, i - from_order + merge_infos[from_order].merge_length);
			}
		};
		for (auto *p_edge : resolved.GetPassNode(i).input_edges)
			test_edge(p_edge);
		for (auto &edge : extra_edge_infos[i].input_edges)
			test_extra_edge(&edge);
	}

	return merge_infos;
}

void RenderGraphScheduler::extract_grouped_passes(const RenderGraphResolver &resolved) {
	const uint32_t kOrderedPassCount = resolved.GetPassNodeCount();

	m_passes.clear();
	m_passes.reserve(kOrderedPassCount);

	uint32_t possible_render_pass_count = 0;
	for (const auto &pass_node : resolved.GetPassNodes())
		if (pass_node.pass->m_p_attachment_data)
			++possible_render_pass_count;

	m_render_passes.clear();
	m_render_passes.reserve(possible_render_pass_count);

	std::vector<RenderPassMergeInfo> merge_infos = _compute_pass_merge_info(resolved);
#ifdef MYVK_RG_DEBUG
	printf("Merge \n");
	for (uint32_t i = 0; i < kOrderedPassCount; ++i) {
		printf("%d (%d %d, %d)\n", merge_infos[i].merge_length, merge_infos[i].area.extent.width,
		       merge_infos[i].area.extent.height, merge_infos[i].area.layers);
	}
	printf("\n");
#endif

	for (uint32_t i = 0, prev_length = 0; i < kOrderedPassCount; ++i) {
		const PassBase *pass = resolved.GetPassNode(i).pass;

		auto &length = merge_infos[i].merge_length;
		if (length > prev_length)
			length = prev_length + 1;
		else
			length = pass->m_p_attachment_data ? 1 : 0;

		if (length <= 1) {
			pass->m_scheduled_info.pass_id = m_passes.size();
			pass->m_scheduled_info.subpass_id = 0;
			m_passes.emplace_back();
			auto &pass_info = m_passes.back();
			pass_info.subpasses.push_back({pass});
			if (length == 1) {
				m_render_passes.emplace_back();
				auto &render_pass_info = m_render_passes.back();
				render_pass_info.area = merge_infos[i].area;

				pass_info.p_render_pass_info = &render_pass_info;
			}
		} else {
			pass->m_scheduled_info.pass_id = m_passes.size() - 1;
			pass->m_scheduled_info.subpass_id = length - 1;
			m_passes.back().subpasses.push_back({pass});
		}

		prev_length = length;
	}
}

void RenderGraphScheduler::extract_dependencies(const RenderGraphResolver &resolved) {
	std::vector<std::unordered_map<const ResourceBase *, PassDependency *>> input_dep_maps(m_passes.size()),
	    output_dep_maps(m_passes.size());
	std::unordered_map<const ResourceBase *, PassDependency *> src_output_dep_map, dst_input_dep_map;

	const auto maintain_dependency = [this, &input_dep_maps, &output_dep_maps, &dst_input_dep_map,
	                                  &src_output_dep_map](const RenderGraphResolver::PassEdge &edge) {
		const auto &link_from = edge.from, &link_to = edge.to;
		const auto *resource = edge.resource;

		auto &dep_map_from = link_from.pass ? output_dep_maps[GetPassID(link_from.pass)] : src_output_dep_map,
		     dep_map_to = link_to.pass ? input_dep_maps[GetPassID(link_to.pass)] : dst_input_dep_map;

		auto it_from = dep_map_from.find(resource), it_to = dep_map_to.find(resource);
		assert(it_from == dep_map_from.end() || it_to == dep_map_to.end());

		PassDependency *p_dep;
		if (it_from != dep_map_from.end()) {
			p_dep = it_from->second;
			p_dep->to.emplace_back(link_to);
			dep_map_to[resource] = p_dep;
		} else if (it_to != dep_map_to.end()) {
			p_dep = it_to->second;
			p_dep->from.emplace_back(link_from);
			dep_map_from[resource] = p_dep;
		} else {
			m_pass_dependencies.push_back(
			    {resource, {ResourceReference{link_from}}, {ResourceReference{link_to}}, edge.type});
			p_dep = &m_pass_dependencies.back();
			dep_map_from[resource] = p_dep;
			dep_map_to[resource] = p_dep;
		}
		assert(p_dep->type == edge.type);
	};

	m_pass_dependencies.clear();
	m_pass_dependencies.reserve(resolved.GetPassEdges().size()); // Ensure the pointers are valid

	for (auto &edge : resolved.GetPassEdges()) {
		// Add Dependencies
		const PassBase *pass_from = edge.from.pass;
		const PassBase *pass_to = edge.to.pass;

		if (pass_from && pass_to && GetPassID(pass_from) == GetPassID(pass_to)) {
			assert(GetSubpassID(pass_from) != GetSubpassID(pass_to));
			assert(UsageIsAttachment(edge.from.p_input->GetUsage()) && UsageIsAttachment(edge.to.p_input->GetUsage()));
			m_passes[GetPassID(pass_to)].p_render_pass_info->subpass_dependencies.push_back(
			    {edge.resource, ResourceReference{edge.from}, ResourceReference{edge.to}});
		} else
			maintain_dependency(edge);
	}
}

void RenderGraphScheduler::sort_and_insert_image_dependencies() {
	uint32_t origin_dependency_count = m_pass_dependencies.size();
	for (uint32_t i = 0; i < origin_dependency_count; ++i) {
		auto &dep = m_pass_dependencies[i];
		// WARNING: dep might be invalid after push_back()

		if (dep.type == DependencyType::kExternal) {
			continue;
		}

		assert(dep.from.size() == 1 && !dep.to.empty());

		// Sort the outputs and cull the useless ones
		std::sort(dep.to.begin(), dep.to.end(), [](const ResourceReference &l, const ResourceReference &r) {
			return GetPassID(l.pass) < GetPassID(r.pass);
		});

		if (dep.resource->GetType() == ResourceType::kImage) {
			std::vector<ResourceReference> links = std::move(dep.to);
			dep.to.clear();

			PassDependency *p_cur_dep = &dep;

			for (const auto &link : links) {
				if (link.p_input && !p_cur_dep->to.empty()) {
					const auto &prev_link = p_cur_dep->to.back();
					bool prev_is_attachment = UsageIsAttachment(prev_link.p_input->GetUsage());
					bool image_layout_changed = UsageGetImageLayout(prev_link.p_input->GetUsage()) !=
					                            UsageGetImageLayout(link.p_input->GetUsage());
					bool is_write_or_result = !UsageIsReadOnly(link.p_input->GetUsage());
					assert(!is_write_or_result || link.pass == links.back().pass);
					if (prev_is_attachment || image_layout_changed || is_write_or_result) {
						m_pass_dependencies.push_back({p_cur_dep->resource, p_cur_dep->to,
						                               std::vector<ResourceReference>{}, DependencyType::kDependency});
						p_cur_dep = &m_pass_dependencies.back();
					}
				}
				p_cur_dep->to.push_back(link);
				if (!link.p_input) {
					assert(link.pass == nullptr && links.size() == 1);
					break;
				}
			}
			if (UsageIsReadOnly(p_cur_dep->to.back().p_input->GetUsage()) &&
			    p_cur_dep->resource->GetState() == ResourceState::kExternal) {
				m_pass_dependencies.push_back({p_cur_dep->resource, p_cur_dep->to, {{}}, DependencyType::kExternal});
			}
		}
	}
}

void RenderGraphScheduler::extract_pass_attachments() {
	for (auto &pass_info : m_passes) {
		auto &attachment_id_map = pass_info.p_render_pass_info->attachment_id_map;

		const auto register_attachment = [&attachment_id_map](const ImageBase *image) {
			if (attachment_id_map.find(image) == attachment_id_map.end()) {
				uint32_t id = attachment_id_map.size();
				attachment_id_map[image] = id;
			}
		};

		for (const auto &subpass_info : pass_info.subpasses) {
			subpass_info.pass->for_each_input([&register_attachment](const Input *p_input) {
				if (UsageIsAttachment(p_input->GetUsage())) {
					p_input->GetResource()->Visit([&register_attachment](const auto *image) {
						if constexpr (ResourceVisitorTrait<decltype(image)>::kType == ResourceType::kImage) {
							if constexpr (ResourceVisitorTrait<decltype(image)>::kIsAlias)
								register_attachment(image->GetPointedResource());
							else
								register_attachment(image);
						} else
							assert(false);
					});
				}
			});
		}
	}
}

void RenderGraphScheduler::extract_resource_transient_info() {
	// TODO: Implement this
}

void RenderGraphScheduler::Schedule(const RenderGraphResolver &resolved) {
	extract_grouped_passes(resolved);
	extract_dependencies(resolved);
	sort_and_insert_image_dependencies();
	extract_pass_attachments();

#ifdef MYVK_RG_DEBUG
	printf("\nResolved Passes: \n");
	for (const auto &pass_info : m_passes) {
		printf("PASS #%u (isRenderPass = %d): ", GetPassID(pass_info.subpasses[0].pass),
		       pass_info.p_render_pass_info ? 1 : 0);
		for (const auto &subpass_info : pass_info.subpasses)
			std::cout << subpass_info.pass->GetKey().GetName() << ":" << subpass_info.pass->GetKey().GetID() << ", ";
		printf("\n");
		if (pass_info.p_render_pass_info) {
			printf("RENDER AREA: {%d x %d, %d}\n", pass_info.p_render_pass_info->area.extent.width,
			       pass_info.p_render_pass_info->area.extent.height, pass_info.p_render_pass_info->area.layers);
			printf("PASS ATTACHMENTS: ");
			for (const auto &attachment : pass_info.p_render_pass_info->attachment_id_map)
				std::cout << attachment.first->GetKey().GetName() << ":" << attachment.first->GetKey().GetID()
				          << " id = " << attachment.second << ", ";
			printf("\n");
		}
		printf("\n");
	}
	printf("\n");
#endif
}

} // namespace myvk_rg::_details_

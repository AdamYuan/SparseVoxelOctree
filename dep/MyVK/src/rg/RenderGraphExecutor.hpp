#ifndef MYVK_RG_RENDER_GRAPH_EXECUTOR_HPP
#define MYVK_RG_RENDER_GRAPH_EXECUTOR_HPP

#include "RenderGraphAllocator.hpp"
#include "RenderGraphScheduler.hpp"

#include <myvk/ImagelessFramebuffer.hpp>
#include <myvk/RenderPass.hpp>

#include <algorithm>

namespace myvk_rg::_details_ {

class RenderGraphExecutor {
private:
	myvk::Ptr<myvk::Device> m_device_ptr;
	const RenderGraphResolver *m_p_resolved;
	const RenderGraphScheduler *m_p_scheduled;
	const RenderGraphAllocator *m_p_allocated;

	struct MemoryBarrier {
		VkPipelineStageFlags2 src_stage_mask;
		VkAccessFlags2 src_access_mask;
		VkPipelineStageFlags2 dst_stage_mask;
		VkAccessFlags2 dst_access_mask;
	};
	struct BufferMemoryBarrier : public MemoryBarrier {
		const BufferBase *buffer;
		inline bool is_valid_buffer_barrier() const {
			return (src_stage_mask | src_access_mask) && (dst_stage_mask | dst_access_mask);
		}
	};
	struct ImageMemoryBarrier : public MemoryBarrier {
		const ImageBase *image;
		VkImageLayout old_layout;
		VkImageLayout new_layout;
		inline bool is_valid_image_barrier() const {
			return ((src_stage_mask | src_access_mask) && (dst_stage_mask | dst_access_mask)) ||
			       old_layout != new_layout;
		}
	};

	struct AttachmentInfo {
		struct AttachmentReference {
			const Input *p_input{};
			uint32_t subpass{};
		};
		const ImageBase *image{};
		std::vector<AttachmentReference> references;
	};
	struct RenderPassInfo {
		myvk::Ptr<myvk::RenderPass> myvk_render_pass;
		myvk::Ptr<myvk::ImagelessFramebuffer> myvk_framebuffer;
		std::vector<AttachmentInfo> attachments;
	};
	struct BarrierInfo {
		std::vector<BufferMemoryBarrier> buffer_barriers;
		std::vector<ImageMemoryBarrier> image_barriers;

		inline bool empty() const { return buffer_barriers.empty() && image_barriers.empty(); }
		inline void clear() {
			buffer_barriers.clear();
			image_barriers.clear();
		}
	};
	struct PassExecutor {
		const RenderGraphScheduler::PassInfo *p_info{};
		BarrierInfo prior_barrier_info;
		RenderPassInfo render_pass_info;
	};
	std::vector<PassExecutor> m_pass_executors;
	BarrierInfo m_post_barrier_info;

	struct SubpassDependencies;
	struct DependencyBuilder;

	void reset_pass_executor_vector();
	void reset_pass_pipeline_state();
	void _process_validation_dependency(const RenderGraphScheduler::PassDependency &dep,
	                                    std::vector<SubpassDependencies> *p_sub_deps);
	void _process_generic_dependency(const RenderGraphScheduler::PassDependency &dep,
	                                 std::vector<SubpassDependencies> *p_sub_deps);
	void _process_external_dependency(const RenderGraphScheduler::PassDependency &dep,
	                                  std::vector<SubpassDependencies> *p_sub_deps);
	void _process_last_frame_dependency(const RenderGraphScheduler::PassDependency &dep,
	                                    std::vector<SubpassDependencies> *p_sub_deps);
	std::vector<SubpassDependencies> extract_barriers_and_subpass_dependencies();
	void create_render_passes_and_framebuffers(std::vector<SubpassDependencies> &&subpass_dependencies);

public:
	void Prepare(const myvk::Ptr<myvk::Device> &device, const RenderGraphResolver &resolved,
	             const RenderGraphScheduler &scheduled, const RenderGraphAllocator &allocated);

	inline const auto &GetPassExec(uint32_t pass_id) const { return m_pass_executors[pass_id]; }
	inline const auto &GetPassExec(const PassBase *pass) const {
		return m_pass_executors[RenderGraphScheduler::GetPassID(pass)];
	}

	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer, bool flip) const;
};

} // namespace myvk_rg::_details_

#endif

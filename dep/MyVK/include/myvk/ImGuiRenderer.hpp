#ifdef MYVK_ENABLE_IMGUI

#ifndef MYVK_IMGUI_RENDERER_HPP
#define MYVK_IMGUI_RENDERER_HPP

#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSet.hpp"
#include "GraphicsPipeline.hpp"
#include "Image.hpp"
#include "ImageView.hpp"
#include "Sampler.hpp"

namespace myvk {
class ImGuiRenderer : public Base {
private:
	Ptr<DescriptorSetLayout> m_descriptor_set_layout;
	Ptr<DescriptorPool> m_descriptor_pool;
	Ptr<DescriptorSet> m_descriptor_set;

	Ptr<PipelineLayout> m_pipeline_layout;
	Ptr<GraphicsPipeline> m_pipeline;

	mutable std::vector<Ptr<Buffer>> m_vertex_buffers, m_index_buffers;

	void create_descriptor(const Ptr<Device> &device);

	void create_pipeline(const Ptr<RenderPass> &render_pass, uint32_t subpass);

	void setup_render_state(const Ptr<CommandBuffer> &command_buffer, int fb_width, int fb_height,
	                        uint32_t current_frame) const;

	void initialize(const Ptr<RenderPass> &render_pass, uint32_t subpass, uint32_t frame_count);

public:
	inline ~ImGuiRenderer() override = default;
	inline static Ptr<ImGuiRenderer> Create(const Ptr<RenderPass> &render_pass, uint32_t subpass,
	                                        uint32_t frame_count) {
		auto ret = std::make_shared<ImGuiRenderer>();
		ret->initialize(render_pass, subpass, frame_count);
		return ret;
	}

	void CmdDrawPipeline(const Ptr<CommandBuffer> &command_buffer, uint32_t current_frame) const;
};
} // namespace myvk

#endif

#endif
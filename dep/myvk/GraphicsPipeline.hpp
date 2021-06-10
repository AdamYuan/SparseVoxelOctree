#ifndef MYVK_GRAPHICS_PIPELINE_HPP
#define MYVK_GRAPHICS_PIPELINE_HPP

#include "PipelineBase.hpp"
#include "RenderPass.hpp"

#include <memory>

namespace myvk {
class GraphicsPipeline : public PipelineBase {
private:
	std::shared_ptr<RenderPass> m_render_pass_ptr;

public:
	static std::shared_ptr<GraphicsPipeline> Create(const std::shared_ptr<PipelineLayout> &pipeline_layout,
	                                                const std::shared_ptr<RenderPass> &render_pass,
	                                                const VkGraphicsPipelineCreateInfo &create_info);

	VkPipelineBindPoint GetBindPoint() const override { return VK_PIPELINE_BIND_POINT_GRAPHICS; }

	const std::shared_ptr<RenderPass> &GetRenderPassPtr() const { return m_render_pass_ptr; }
};
} // namespace myvk

#endif

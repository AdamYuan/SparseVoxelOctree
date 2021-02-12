#ifndef PATH_TRACER_VIEWER_HPP
#define PATH_TRACER_VIEWER_HPP

#include "PathTracer.hpp"
#include "myvk/Framebuffer.hpp"
#include "myvk/GraphicsPipeline.hpp"
#include "myvk/Image.hpp"

class PathTracerViewer {
public:
	enum class ViewTypes { kColor = 0, kAlbedo, kNormal } m_view_type = ViewTypes::kColor;

private:
	const PathTracer *m_path_tracer;

	std::shared_ptr<myvk::Image> m_image;
	std::shared_ptr<myvk::ImageView> m_image_view;
	std::shared_ptr<myvk::Sampler> m_sampler;

	std::shared_ptr<myvk::RenderPass> m_gen_render_pass;
	std::shared_ptr<myvk::Framebuffer> m_gen_framebuffer;
	std::shared_ptr<myvk::PipelineLayout> m_gen_pipeline_layout;
	std::shared_ptr<myvk::GraphicsPipeline> m_gen_graphics_pipeline;

	std::shared_ptr<myvk::DescriptorPool> m_descriptor_pool;
	std::shared_ptr<myvk::DescriptorSetLayout> m_descriptor_set_layout;
	std::shared_ptr<myvk::DescriptorSet> m_descriptor_set;
	std::shared_ptr<myvk::PipelineLayout> m_main_pipeline_layout;
	std::shared_ptr<myvk::GraphicsPipeline> m_main_graphics_pipeline;

	void create_render_pass(const std::shared_ptr<myvk::Device> &device, VkFormat format);
	void create_gen_graphics_pipeline(const std::shared_ptr<myvk::Device> &device);
	void create_descriptors(const std::shared_ptr<myvk::Device> &device);
	void create_main_graphics_pipeline(const std::shared_ptr<myvk::RenderPass> &render_pass, uint32_t subpass);

public:
	void Initialize(const PathTracer &path_tracer, const std::shared_ptr<myvk::Swapchain> &swapchain,
	                const std::shared_ptr<myvk::RenderPass> &render_pass, uint32_t subpass);

	void CmdGenRenderPass(const std::shared_ptr<myvk::CommandBuffer> &command_buffer) const;

	void CmdDrawPipeline(const std::shared_ptr<myvk::CommandBuffer> &command_buffer) const;

	const std::shared_ptr<myvk::Image> &GetImage() const { return m_image; }
};

#endif
#ifndef OCTREE_TRACER_HPP
#define OCTREE_TRACER_HPP

#include "Camera.hpp"
#include "Lighting.hpp"
#include "Octree.hpp"
#include "myvk/Buffer.hpp"
#include "myvk/CommandBuffer.hpp"
#include "myvk/DescriptorSet.hpp"
#include "myvk/GraphicsPipeline.hpp"
#include "myvk/Image.hpp"

class OctreeTracer {
public:
	enum class ViewTypes { kDiffuse = 0, kNormal, kIteration } m_view_type = ViewTypes::kDiffuse;
	bool m_beam_enable{true};

private:
	uint32_t m_width{kDefaultWidth}, m_height{kDefaultHeight};

	std::shared_ptr<Octree> m_octree_ptr;
	std::shared_ptr<Camera> m_camera_ptr;
	std::shared_ptr<Lighting> m_lighting_ptr;

	std::shared_ptr<myvk::DescriptorPool> m_descriptor_pool;
	std::shared_ptr<myvk::DescriptorSetLayout> m_descriptor_set_layout;

	std::shared_ptr<myvk::PipelineLayout> m_main_pipeline_layout;
	std::shared_ptr<myvk::PipelineLayout> m_beam_pipeline_layout;

	std::shared_ptr<myvk::RenderPass> m_beam_render_pass;
	std::shared_ptr<myvk::GraphicsPipeline> m_beam_graphics_pipeline;

	std::shared_ptr<myvk::GraphicsPipeline> m_main_graphics_pipeline;
	struct FrameResource {
		std::shared_ptr<myvk::Image> m_beam_image;
		std::shared_ptr<myvk::ImageView> m_beam_image_view;
		std::shared_ptr<myvk::Framebuffer> m_beam_framebuffer;
		std::shared_ptr<myvk::Sampler> m_beam_sampler;
		std::shared_ptr<myvk::DescriptorSet> m_descriptor_set;
	};
	std::vector<FrameResource> m_frame_resources;

	void create_descriptor_pool(const std::shared_ptr<myvk::Device> &device, uint32_t frame_count);

	void create_layouts(const std::shared_ptr<myvk::Device> &device);

	void create_beam_render_pass(const std::shared_ptr<myvk::Device> &device);

	void create_frame_resources(const std::shared_ptr<myvk::Device> &device, uint32_t frame_count);

	void create_main_graphics_pipeline(const std::shared_ptr<myvk::RenderPass> &render_pass, uint32_t subpass);

	void create_beam_graphics_pipeline(const std::shared_ptr<myvk::Device> &device);

public:
	static std::shared_ptr<OctreeTracer> Create(const std::shared_ptr<Octree> &octree,
	                                            const std::shared_ptr<Camera> &camera,
	                                            const std::shared_ptr<Lighting> &lighting,
	                                            const std::shared_ptr<myvk::RenderPass> &render_pass, uint32_t subpass,
	                                            uint32_t frame_count);
	const std::shared_ptr<Octree> &GetOctreePtr() const { return m_octree_ptr; }
	const std::shared_ptr<Camera> &GetCameraPtr() const { return m_camera_ptr; }
	const std::shared_ptr<Lighting> &GetLightingPtr() const { return m_lighting_ptr; }

	void Resize(uint32_t width, uint32_t height);

	void CmdBeamRenderPass(const std::shared_ptr<myvk::CommandBuffer> &command_buffer, uint32_t current_frame) const;

	void CmdDrawPipeline(const std::shared_ptr<myvk::CommandBuffer> &command_buffer, uint32_t current_frame) const;
};

#endif

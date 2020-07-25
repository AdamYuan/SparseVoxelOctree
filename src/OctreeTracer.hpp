#ifndef OCTREE_TRACER_HPP
#define OCTREE_TRACER_HPP

#include "myvk/Buffer.hpp"
#include "myvk/DescriptorSet.hpp"
#include "myvk/GraphicsPipeline.hpp"
#include "myvk/ComputePipeline.hpp"
#include "myvk/CommandBuffer.hpp"
#include "Camera.hpp"
#include "Octree.hpp"
#include "myvk/Image.hpp"

class OctreeTracer {
public:
	enum ViewTypeEnum { kDiffuse = 0, kNormal, kIteration } m_view_type = kDiffuse;
	bool m_beam_enable{true};
	float m_beam_dir_size{0.0f}, m_beam_origin_size{0.008f};

private:
	std::shared_ptr<myvk::DescriptorPool> m_descriptor_pool;
	std::shared_ptr<myvk::DescriptorSetLayout> m_descriptor_set_layout;
	std::shared_ptr<myvk::PipelineLayout> m_pipeline_layout;
	std::shared_ptr<myvk::GraphicsPipeline> m_graphics_pipeline;

	struct FrameResource {
		std::shared_ptr<myvk::Image> m_beam_image;
		std::shared_ptr<myvk::ImageView> m_beam_image_view;
		std::shared_ptr<myvk::Sampler> m_beam_sampler;
		std::shared_ptr<myvk::DescriptorSet> m_descriptor_set;
	};
	std::vector<FrameResource> m_frame_resources;

	const Octree *m_octree;
	const Camera *m_camera;

	void create_descriptor_pool(const std::shared_ptr<myvk::Device> &device, uint32_t frame_count);

	void create_layouts(const std::shared_ptr<myvk::Device> &device);

	void create_frame_resources(const std::shared_ptr<myvk::Device> &device, uint32_t frame_count);

	void create_main_graphics_pipeline(const std::shared_ptr<myvk::RenderPass> &render_pass, uint32_t subpass,
									   VkExtent2D extent);

public:
	void
	Initialize(const Octree &octree, const Camera &camera, const std::shared_ptr<myvk::RenderPass> &render_pass,
			   uint32_t subpass, uint32_t frame_count);

	void CmdCompute(const std::shared_ptr<myvk::CommandBuffer> &command_buffer, uint32_t current_frame) const;

	void CmdDrawPipeline(const std::shared_ptr<myvk::CommandBuffer> &command_buffer, uint32_t current_frame) const;
};


#endif

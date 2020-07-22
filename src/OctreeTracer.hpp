#ifndef OCTREE_TRACER_HPP
#define OCTREE_TRACER_HPP

#include "myvk/Buffer.hpp"
#include "myvk/DescriptorSet.hpp"
#include "myvk/GraphicsPipeline.hpp"
#include "myvk/CommandBuffer.hpp"
#include "Camera.hpp"
#include "Octree.hpp"

class OctreeTracer {
public:
	enum ViewTypeEnum { kDiffuse = 0, kNormal, kIteration } m_view_type = kDiffuse;
	bool m_beam_enable{true};
	float m_beam_dir_size{0.0f}, m_beam_origin_size{0.008f};

private:
	std::shared_ptr<myvk::DescriptorPool> m_descriptor_pool;
	std::shared_ptr<myvk::DescriptorSetLayout> m_descriptor_set_layout;

	std::shared_ptr<myvk::PipelineLayout> m_pipeline_layout;
	std::shared_ptr<myvk::GraphicsPipeline> m_pipeline;

	//per-frame resources
	std::vector<std::shared_ptr<myvk::Buffer>> m_camera_uniform_buffers;
	std::vector<std::shared_ptr<myvk::DescriptorSet>> m_descriptor_sets;

	void create_uniform_buffers(const std::shared_ptr<myvk::Device> &device, uint32_t frame_count);

	void create_descriptors(const std::shared_ptr<myvk::Device> &device, uint32_t frame_count);

	void create_pipeline(const std::shared_ptr<myvk::RenderPass> &render_pass, uint32_t subpass,
						 VkExtent2D extent);

public:
	void
	Initialize(const std::shared_ptr<myvk::Swapchain> &swapchain, const std::shared_ptr<myvk::RenderPass> &render_pass,
			   uint32_t subpass, uint32_t frame_count);

	void UpdateOctree(const Octree &octree);

	void CmdDrawPipeline(const std::shared_ptr<myvk::CommandBuffer> &command_buffer, const Camera &camera, uint32_t current_frame) const;
};


#endif

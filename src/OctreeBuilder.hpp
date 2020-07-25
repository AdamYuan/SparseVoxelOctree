#ifndef OCTREE_BUILDER_HPP
#define OCTREE_BUILDER_HPP

#include "Voxelizer.hpp"
#include "AtomicCounter.hpp"

#include "myvk/RenderPass.hpp"
#include "myvk/Framebuffer.hpp"
#include "myvk/ComputePipeline.hpp"
#include "myvk/Buffer.hpp"
#include "myvk/DescriptorSet.hpp"


class OctreeBuilder {
private:
	std::shared_ptr<myvk::PipelineLayout> m_pipeline_layout;
	std::shared_ptr<myvk::ComputePipeline> m_tag_node_pipeline, m_init_node_pipeline, m_alloc_node_pipeline, m_modify_arg_pipeline;

	AtomicCounter m_atomic_counter;

	std::shared_ptr<myvk::Buffer> m_octree_buffer;
	std::shared_ptr<myvk::Buffer> m_build_info_buffer, m_build_info_staging_buffer;
	std::shared_ptr<myvk::Buffer> m_indirect_buffer, m_indirect_staging_buffer;

	std::shared_ptr<myvk::DescriptorPool> m_descriptor_pool;
	std::shared_ptr<myvk::DescriptorSetLayout> m_descriptor_set_layout;
	std::shared_ptr<myvk::DescriptorSet> m_descriptor_set;

	const Voxelizer *m_voxelizer;

	uint32_t m_octree_level;

	void create_buffers(const std::shared_ptr<myvk::Device> &device);
	void create_descriptors(const std::shared_ptr<myvk::Device> &device);
	void create_pipeline(const std::shared_ptr<myvk::Device> &device);

public:
	void Initialize(const Voxelizer &voxelizer, const std::shared_ptr<myvk::CommandPool> &command_pool,
					uint32_t octree_level);
	void CmdBuild(const std::shared_ptr<myvk::CommandBuffer> &command_buffer) const;
	const std::shared_ptr<myvk::Buffer> &GetOctree() const { return m_octree_buffer; }
};


#endif

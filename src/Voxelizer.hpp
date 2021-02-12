#ifndef VOXELIZER_HPP
#define VOXELIZER_HPP

#include "Counter.hpp"
#include "Scene.hpp"

#include "myvk/Buffer.hpp"
#include "myvk/DescriptorSet.hpp"
#include "myvk/Framebuffer.hpp"
#include "myvk/GraphicsPipeline.hpp"
#include "myvk/RenderPass.hpp"

class Voxelizer {
private:
	std::shared_ptr<Scene> m_scene_ptr;

	std::shared_ptr<myvk::RenderPass> m_render_pass;
	std::shared_ptr<myvk::Framebuffer> m_framebuffer;

	std::shared_ptr<myvk::PipelineLayout> m_pipeline_layout;
	std::shared_ptr<myvk::GraphicsPipeline> m_pipeline;

	Counter m_atomic_counter;

	std::shared_ptr<myvk::Buffer> m_voxel_fragment_list;

	std::shared_ptr<myvk::DescriptorPool> m_descriptor_pool;
	std::shared_ptr<myvk::DescriptorSetLayout> m_descriptor_set_layout;
	std::shared_ptr<myvk::DescriptorSet> m_descriptor_set;

	uint32_t m_level, m_voxel_resolution, m_voxel_fragment_count;

	void create_descriptors(const std::shared_ptr<myvk::Device> &device);
	void create_render_pass(const std::shared_ptr<myvk::Device> &device);
	void create_pipeline(const std::shared_ptr<myvk::Device> &device);
	void count_and_create_fragment_list(const std::shared_ptr<myvk::CommandPool> &command_pool);

public:
	static std::shared_ptr<Voxelizer> Create(const std::shared_ptr<Scene> &scene,
	                                         const std::shared_ptr<myvk::CommandPool> &command_pool,
	                                         uint32_t octree_level);
	const std::shared_ptr<Scene> &GetScenePtr() const { return m_scene_ptr; }
	uint32_t GetLevel() const { return m_level; }

	void CmdVoxelize(const std::shared_ptr<myvk::CommandBuffer> &command_buffer) const;
	uint32_t GetVoxelResolution() const { return m_voxel_resolution; }
	uint32_t GetVoxelFragmentCount() const { return m_voxel_fragment_count; }
	const std::shared_ptr<myvk::Buffer> &GetVoxelFragmentList() const { return m_voxel_fragment_list; }
};

#endif

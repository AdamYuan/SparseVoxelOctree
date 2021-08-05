#include "OctreeBuilder.hpp"
#include "Config.hpp"

#include <spdlog/spdlog.h>

inline static constexpr uint32_t group_x_64(uint32_t x) { return (x >> 6u) + ((x & 0x3fu) ? 1u : 0u); }

std::shared_ptr<OctreeBuilder> OctreeBuilder::Create(const std::shared_ptr<Voxelizer> &voxelizer,
                                                     const std::shared_ptr<myvk::CommandPool> &command_pool) {
	std::shared_ptr<OctreeBuilder> ret = std::make_shared<OctreeBuilder>();

	std::shared_ptr<myvk::Device> device = command_pool->GetDevicePtr();
	ret->m_voxelizer_ptr = voxelizer;
	ret->m_atomic_counter.Initialize(device);
	ret->m_atomic_counter.Reset(command_pool, 0);

	ret->create_buffers(device);
	ret->create_descriptors(device);
	ret->create_pipeline(device);

	return ret;
}

void OctreeBuilder::create_buffers(const std::shared_ptr<myvk::Device> &device) {
	m_build_info_buffer = myvk::Buffer::Create(device, 2 * sizeof(uint32_t), VMA_MEMORY_USAGE_GPU_ONLY,
	                                           VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	m_build_info_staging_buffer = myvk::Buffer::CreateStaging(device, m_build_info_buffer->GetSize());
	{
		auto *data = (uint32_t *)m_build_info_staging_buffer->Map();
		data[0] = 0; // uAllocBegin
		data[1] = 8; // uAllocNum
		m_build_info_staging_buffer->Unmap();
	}

	m_indirect_buffer = myvk::Buffer::Create(device, 3 * sizeof(uint32_t), VMA_MEMORY_USAGE_GPU_ONLY,
	                                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
	                                             VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	m_indirect_staging_buffer = myvk::Buffer::CreateStaging(device, m_indirect_buffer->GetSize());
	{
		auto *data = (uint32_t *)m_indirect_staging_buffer->Map();
		data[0] = 1; // uGroupX
		data[1] = 1; // uGroupY
		data[2] = 1; // uGroupZ
		m_indirect_staging_buffer->Unmap();
	}

	// Estimate octree buffer size
	uint32_t octree_node_num = std::max(kOctreeNodeNumMin, m_voxelizer_ptr->GetVoxelFragmentCount() << 2u);
	octree_node_num = std::min(octree_node_num, kOctreeNodeNumMax);

	m_octree_buffer = myvk::Buffer::Create(device, octree_node_num * sizeof(uint32_t), VMA_MEMORY_USAGE_GPU_ONLY,
	                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	spdlog::info("Octree buffer created with {} nodes ({} MB)", octree_node_num,
	             m_octree_buffer->GetSize() / 1000000.0);
}

void OctreeBuilder::create_descriptors(const std::shared_ptr<myvk::Device> &device) {
	m_descriptor_pool = myvk::DescriptorPool::Create(device, 1, {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 5}});
	{
		VkDescriptorSetLayoutBinding atomic_counter_binding = {};
		atomic_counter_binding.binding = 0;
		atomic_counter_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		atomic_counter_binding.descriptorCount = 1;
		atomic_counter_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding octree_binding = {};
		octree_binding.binding = 1;
		octree_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		octree_binding.descriptorCount = 1;
		octree_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding fragment_list_binding = {};
		fragment_list_binding.binding = 2;
		fragment_list_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		fragment_list_binding.descriptorCount = 1;
		fragment_list_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding build_info_binding = {};
		build_info_binding.binding = 3;
		build_info_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		build_info_binding.descriptorCount = 1;
		build_info_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding indirect_binding = {};
		indirect_binding.binding = 4;
		indirect_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		indirect_binding.descriptorCount = 1;
		indirect_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		m_descriptor_set_layout =
		    myvk::DescriptorSetLayout::Create(device, {atomic_counter_binding, octree_binding, fragment_list_binding,
		                                               build_info_binding, indirect_binding});
	}
	m_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_descriptor_set_layout);
	m_descriptor_set->UpdateStorageBuffer(m_atomic_counter.GetBuffer(), 0);
	m_descriptor_set->UpdateStorageBuffer(m_octree_buffer, 1);
	m_descriptor_set->UpdateStorageBuffer(m_voxelizer_ptr->GetVoxelFragmentList(), 2);
	m_descriptor_set->UpdateStorageBuffer(m_build_info_buffer, 3);
	m_descriptor_set->UpdateStorageBuffer(m_indirect_buffer, 4);
}

void OctreeBuilder::create_pipeline(const std::shared_ptr<myvk::Device> &device) {
	m_pipeline_layout = myvk::PipelineLayout::Create(device, {m_descriptor_set_layout},
	                                                 {{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t) * 2}});

	{
		constexpr uint32_t kOctreeTagNodeCompSpv[] = {
#include "spirv/octree_tag_node.comp.u32"
		};
		std::shared_ptr<myvk::ShaderModule> octree_tag_node_shader_module =
		    myvk::ShaderModule::Create(device, kOctreeTagNodeCompSpv, sizeof(kOctreeTagNodeCompSpv));
		m_tag_node_pipeline = myvk::ComputePipeline::Create(m_pipeline_layout, octree_tag_node_shader_module);
	}

	{
		constexpr uint32_t kOctreeInitNodeCompSpv[] = {
#include "spirv/octree_init_node.comp.u32"
		};
		std::shared_ptr<myvk::ShaderModule> octree_init_node_shader_module =
		    myvk::ShaderModule::Create(device, kOctreeInitNodeCompSpv, sizeof(kOctreeInitNodeCompSpv));
		m_init_node_pipeline = myvk::ComputePipeline::Create(m_pipeline_layout, octree_init_node_shader_module);
	}

	{
		constexpr uint32_t kOctreeAllocNodeCompSpv[] = {
#include "spirv/octree_alloc_node.comp.u32"
		};
		std::shared_ptr<myvk::ShaderModule> octree_alloc_node_shader_module =
		    myvk::ShaderModule::Create(device, kOctreeAllocNodeCompSpv, sizeof(kOctreeAllocNodeCompSpv));
		m_alloc_node_pipeline = myvk::ComputePipeline::Create(m_pipeline_layout, octree_alloc_node_shader_module);
	}

	{
		constexpr uint32_t kOctreeModifyArgCompSpv[] = {
#include "spirv/octree_modify_arg.comp.u32"
		};
		std::shared_ptr<myvk::ShaderModule> octree_modify_arg_shader_module =
		    myvk::ShaderModule::Create(device, kOctreeModifyArgCompSpv, sizeof(kOctreeModifyArgCompSpv));
		m_modify_arg_pipeline = myvk::ComputePipeline::Create(m_pipeline_layout, octree_modify_arg_shader_module);
	}
}

void OctreeBuilder::CmdBuild(const std::shared_ptr<myvk::CommandBuffer> &command_buffer) const {
	// transfers
	{
		command_buffer->CmdCopy(m_build_info_staging_buffer, m_build_info_buffer,
		                        {{0, 0, m_build_info_buffer->GetSize()}});
		command_buffer->CmdCopy(m_indirect_staging_buffer, m_indirect_buffer, {{0, 0, m_indirect_buffer->GetSize()}});

		command_buffer->CmdPipelineBarrier(
		    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {},
		    {m_build_info_buffer->GetMemoryBarrier(VK_ACCESS_TRANSFER_WRITE_BIT,
		                                           VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)},
		    {});

		command_buffer->CmdPipelineBarrier(
		    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		    {},
		    {m_indirect_buffer->GetMemoryBarrier(VK_ACCESS_TRANSFER_WRITE_BIT,
		                                         VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)},
		    {});
	}

	uint32_t fragment_group_x = group_x_64(m_voxelizer_ptr->GetVoxelFragmentCount());
	uint32_t push_constants[] = {m_voxelizer_ptr->GetVoxelFragmentCount(), m_voxelizer_ptr->GetVoxelResolution()};

	command_buffer->CmdBindDescriptorSets({m_descriptor_set}, m_pipeline_layout, VK_PIPELINE_BIND_POINT_COMPUTE, {});
	command_buffer->CmdPushConstants(m_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, 2 * sizeof(uint32_t),
	                                 push_constants);

	for (uint32_t i = 1; i <= m_voxelizer_ptr->GetLevel(); ++i) {
		command_buffer->CmdBindPipeline(m_init_node_pipeline);
		command_buffer->CmdDispatchIndirect(m_indirect_buffer);

		command_buffer->CmdPipelineBarrier(
		    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {},
		    {m_octree_buffer->GetMemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT,
		                                       VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)},
		    {});

		command_buffer->CmdBindPipeline(m_tag_node_pipeline);
		command_buffer->CmdDispatch(fragment_group_x, 1, 1);

		if (i != m_voxelizer_ptr->GetLevel()) {
			command_buffer->CmdPipelineBarrier(
			    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {},
			    {m_octree_buffer->GetMemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT,
			                                       VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)},
			    {});

			command_buffer->CmdBindPipeline(m_alloc_node_pipeline);
			command_buffer->CmdDispatchIndirect(m_indirect_buffer);

			command_buffer->CmdPipelineBarrier(
			    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {},
			    {m_octree_buffer->GetMemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT,
			                                       VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)},
			    {});

			command_buffer->CmdBindPipeline(m_modify_arg_pipeline);
			command_buffer->CmdDispatch(1, 1, 1);

			command_buffer->CmdPipelineBarrier(
			    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			    VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {},
			    {m_indirect_buffer->GetMemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT,
			                                         VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)},
			    {});
			command_buffer->CmdPipelineBarrier(
			    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {},
			    {m_build_info_buffer->GetMemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT)}, {});
		}
	}
}

VkDeviceSize OctreeBuilder::GetOctreeRange(const std::shared_ptr<myvk::CommandPool> &command_pool) const {
	return (m_atomic_counter.Read(command_pool) + 1u) * 8u * sizeof(uint32_t);
}
void OctreeBuilder::CmdTransferOctreeOwnership(const std::shared_ptr<myvk::CommandBuffer> &command_buffer,
                                               uint32_t src_queue_family, uint32_t dst_queue_family,
                                               VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage) const {
	command_buffer->CmdPipelineBarrier(
	    src_stage, dst_stage, {}, {m_octree_buffer->GetMemoryBarrier(0, 0, src_queue_family, dst_queue_family)}, {});
}

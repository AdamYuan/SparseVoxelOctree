#include "OctreeBuilder.hpp"
#include "OctreeBuilderSpirv.hpp"
#include "Config.hpp"

#include <plog/Log.h>

inline static uint32_t group_x_64(uint32_t x) { return (x >> 6u) + ((x & 0x3fu) ? 1u : 0u); }

void OctreeBuilder::Initialize(const Voxelizer &voxelizer, const std::shared_ptr<myvk::CommandPool> &command_pool,
							   uint32_t octree_level) {
	std::shared_ptr<myvk::Device> device = command_pool->GetDevicePtr();
	m_octree_level = octree_level;
	m_voxelizer = &voxelizer;
	m_atomic_counter.Initialize(device);
	m_atomic_counter.Reset(command_pool, 0);

	create_buffers(device);
	create_descriptors(device);
	create_pipeline(device);
}

void OctreeBuilder::create_buffers(const std::shared_ptr<myvk::Device> &device) {
	m_build_info_buffer = myvk::Buffer::Create(device, 2 * sizeof(uint32_t), VMA_MEMORY_USAGE_GPU_ONLY,
											   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	m_build_info_staging_buffer = myvk::Buffer::CreateStaging(device, m_build_info_buffer->GetSize());
	{
		auto *data = (uint32_t *) m_build_info_staging_buffer->Map();
		data[0] = data[1] = 0; //uAllocBegin, uAllocNum
		m_build_info_staging_buffer->Unmap();
	}

	m_indirect_buffer = myvk::Buffer::Create(device, 3 * sizeof(uint32_t), VMA_MEMORY_USAGE_GPU_ONLY,
											 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
											 VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	m_indirect_staging_buffer = myvk::Buffer::CreateStaging(device, m_indirect_buffer->GetSize());
	{
		auto *data = (uint32_t *) m_indirect_staging_buffer->Map();
		data[0] = 0;
		data[1] = data[2] = 1; //uGroupX, uGroupY, uGroupZ
		m_indirect_staging_buffer->Unmap();
	}

	//Estimate octree buffer size
	uint32_t octree_node_num = std::max(kOctreeNodeNumMin, m_voxelizer->GetVoxelFragmentCount() << 2u);
	octree_node_num = std::min(octree_node_num, kOctreeNodeNumMax);

	m_octree_buffer = myvk::Buffer::Create(device, octree_node_num * sizeof(uint32_t), VMA_MEMORY_USAGE_GPU_ONLY,
										   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	//For clearing octree buffer to 0
	m_octree_staging_buffer = myvk::Buffer::CreateStaging(device, m_octree_buffer->GetSize());
	{
		auto *data = (uint32_t *) m_octree_staging_buffer->Map();
		std::fill(data, data + octree_node_num, 0);
		m_octree_staging_buffer->Unmap();
	}
	LOGV.printf("Octree buffer created with %d nodes (%.1f MB)", octree_node_num, m_octree_buffer->GetSize() / 1000000.0f);
}

void OctreeBuilder::create_descriptors(const std::shared_ptr<myvk::Device> &device) {
	{
		constexpr uint32_t kPoolSizeCount = 1;
		VkDescriptorPoolSize pool_sizes[kPoolSizeCount] = {
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 5}
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1;
		pool_info.poolSizeCount = kPoolSizeCount;
		pool_info.pPoolSizes = pool_sizes;

		m_descriptor_pool = myvk::DescriptorPool::Create(device, pool_info);
	}
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

		m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(
			device,
			{atomic_counter_binding, octree_binding, fragment_list_binding, build_info_binding, indirect_binding});
	}
	m_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_descriptor_set_layout);
	m_descriptor_set->UpdateStorageBuffer(m_atomic_counter.GetBufferPtr(), 0);
	m_descriptor_set->UpdateStorageBuffer(m_octree_buffer, 1);
	m_descriptor_set->UpdateStorageBuffer(m_voxelizer->GetVoxelFragmentListPtr(), 2);
	m_descriptor_set->UpdateStorageBuffer(m_build_info_buffer, 3);
	m_descriptor_set->UpdateStorageBuffer(m_indirect_buffer, 4);
}

void OctreeBuilder::create_pipeline(const std::shared_ptr<myvk::Device> &device) {
	m_pipeline_layout =
		myvk::PipelineLayout::Create(device, {m_descriptor_set_layout},
									 {{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t) * 2}});

	{
		std::shared_ptr<myvk::ShaderModule> octree_tag_node_shader_module =
			myvk::ShaderModule::Create(device, (uint32_t *) kOctreeTagNodeCompSpv, sizeof(kOctreeTagNodeCompSpv));
		m_tag_node_pipeline = myvk::ComputePipeline::Create(m_pipeline_layout, octree_tag_node_shader_module);
	}

	{
		std::shared_ptr<myvk::ShaderModule> octree_alloc_node_shader_module =
			myvk::ShaderModule::Create(device, (uint32_t *) kOctreeAllocNodeCompSpv, sizeof(kOctreeAllocNodeCompSpv));
		m_alloc_node_pipeline = myvk::ComputePipeline::Create(m_pipeline_layout, octree_alloc_node_shader_module);
	}

	{
		std::shared_ptr<myvk::ShaderModule> octree_modify_arg_shader_module =
			myvk::ShaderModule::Create(device, (uint32_t *) kOctreeModifyArgCompSpv, sizeof(kOctreeModifyArgCompSpv));
		m_modify_arg_pipeline = myvk::ComputePipeline::Create(m_pipeline_layout, octree_modify_arg_shader_module);
	}
}

void OctreeBuilder::CmdBuild(const std::shared_ptr<myvk::CommandBuffer> &command_buffer) const {
	{
		//TODO: Use compute shader to dynamically clear octree to 0
		command_buffer->CmdCopy(m_octree_staging_buffer, m_octree_buffer,
								{{0, 0, m_octree_buffer->GetSize()}});
		command_buffer->CmdCopy(m_build_info_staging_buffer, m_build_info_buffer,
								{{0, 0, m_build_info_buffer->GetSize()}});
		command_buffer->CmdCopy(m_indirect_staging_buffer, m_indirect_buffer,
								{{0, 0, m_indirect_buffer->GetSize()}});

		command_buffer->CmdPipelineBarrier(
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			{},
			{
				m_octree_buffer->GetMemoryBarrier({0, m_octree_buffer->GetSize()}, VK_ACCESS_TRANSFER_WRITE_BIT,
												  VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT),
				m_build_info_buffer->GetMemoryBarrier({0, m_build_info_buffer->GetSize()}, VK_ACCESS_TRANSFER_WRITE_BIT,
													  VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT),
				m_indirect_buffer->GetMemoryBarrier({0, m_indirect_buffer->GetSize()}, VK_ACCESS_TRANSFER_WRITE_BIT,
													VK_ACCESS_SHADER_WRITE_BIT)
			},
			{});
	}

	uint32_t fragment_group_x = group_x_64(m_voxelizer->GetVoxelFragmentCount());
	uint32_t push_constants[] = {m_voxelizer->GetVoxelFragmentCount(), m_voxelizer->GetVoxelResolution()};

	command_buffer->CmdBindDescriptorSets({m_descriptor_set}, m_pipeline_layout, VK_PIPELINE_BIND_POINT_COMPUTE, {});
	command_buffer->CmdPushConstants(m_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, 2 * sizeof(uint32_t),
									 push_constants);


	for (uint32_t i = 1; i <= m_octree_level; ++i) {
		command_buffer->CmdBindPipeline(m_tag_node_pipeline);
		command_buffer->CmdDispatch(fragment_group_x, 1, 1);

		if (i != m_octree_level) {
			command_buffer->CmdBindPipeline(m_modify_arg_pipeline);
			command_buffer->CmdDispatch(1, 1, 1);

			command_buffer->CmdPipelineBarrier(
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {}, {
					m_octree_buffer->GetMemoryBarrier({0, m_octree_buffer->GetSize()}, VK_ACCESS_SHADER_WRITE_BIT,
													  VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT),
					m_build_info_buffer->GetMemoryBarrier({0, m_build_info_buffer->GetSize()},
														  VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT)
				}, {});

			command_buffer->CmdPipelineBarrier(
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
				{},
				m_indirect_buffer->GetMemoryBarriers(
					{{0, m_indirect_buffer->GetSize()}},
					VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT),
				{});

			command_buffer->CmdBindPipeline(m_alloc_node_pipeline);
			command_buffer->CmdDispatchIndirect(m_indirect_buffer);

			command_buffer->CmdPipelineBarrier(
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				{},
				m_octree_buffer->GetMemoryBarriers(
					{{0, m_octree_buffer->GetSize()}},
					VK_ACCESS_SHADER_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT),
				{});
		}
	}
}

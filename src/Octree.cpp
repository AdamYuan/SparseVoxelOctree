#include "Octree.hpp"

void Octree::Initialize(const std::shared_ptr<myvk::Device> &device) {
	{
		VkDescriptorSetLayoutBinding octree_binding = {};
		octree_binding.binding = 0;
		octree_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		octree_binding.descriptorCount = 1;
		octree_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(device, {octree_binding});
	}
	{
		VkDescriptorPoolSize pool_sizes[] = {
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1;
		pool_info.poolSizeCount = 1;
		pool_info.pPoolSizes = pool_sizes;

		m_descriptor_pool = myvk::DescriptorPool::Create(device, pool_info);
	}
	m_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_descriptor_set_layout);
}

void Octree::Update(const std::shared_ptr<myvk::Buffer> &buffer, uint32_t level) {
	m_buffer = buffer;
	m_level = level;
	m_descriptor_set->UpdateStorageBuffer(buffer, 0);
}

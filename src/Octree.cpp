#include "Octree.hpp"

void Octree::Initialize(const std::shared_ptr<myvk::Device> &device) {
	{
		VkDescriptorSetLayoutBinding octree_binding = {};
		octree_binding.binding = 0;
		octree_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		octree_binding.descriptorCount = 1;
		octree_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(device, {octree_binding});
	}
	m_descriptor_pool = myvk::DescriptorPool::Create(device, 1, {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}});
	m_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_descriptor_set_layout);
}

void Octree::Update(const std::shared_ptr<myvk::Buffer> &buffer, uint32_t level, VkDeviceSize range) {
	m_buffer = buffer;
	m_level = level;
	m_range = range;
	m_descriptor_set->UpdateStorageBuffer(buffer, 0, 0, 0, range);
}

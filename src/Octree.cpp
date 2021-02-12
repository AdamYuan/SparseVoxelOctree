#include "Octree.hpp"

std::shared_ptr<Octree> Octree::Create(const std::shared_ptr<myvk::Device> &device) {
	std::shared_ptr<Octree> ret = std::make_shared<Octree>();
	{
		VkDescriptorSetLayoutBinding octree_binding = {};
		octree_binding.binding = 0;
		octree_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		octree_binding.descriptorCount = 1;
		octree_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		ret->m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(device, {octree_binding});
	}
	ret->m_descriptor_pool = myvk::DescriptorPool::Create(device, 1, {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}});
	ret->m_descriptor_set = myvk::DescriptorSet::Create(ret->m_descriptor_pool, ret->m_descriptor_set_layout);

	return ret;
}

void Octree::Update(const std::shared_ptr<myvk::CommandPool> &command_pool,
                    const std::shared_ptr<OctreeBuilder> &builder) {
	uint32_t octree_range = builder->GetOctreeRange(command_pool);
	m_buffer = builder->GetOctree();
	m_level = builder->GetLevel();
	m_range = octree_range;
	m_descriptor_set->UpdateStorageBuffer(m_buffer, 0, 0, 0, octree_range);
}

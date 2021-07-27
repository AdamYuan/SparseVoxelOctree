#include "EnvironmentMap.hpp"

std::shared_ptr<EnvironmentMap> EnvironmentMap::Create(const std::shared_ptr<myvk::Device> &device) {
	std::shared_ptr<EnvironmentMap> ret = std::make_shared<EnvironmentMap>();
	ret->create_descriptors(device);
	return ret;
}

void EnvironmentMap::create_descriptors(const std::shared_ptr<myvk::Device> &device) {
	m_descriptor_pool = myvk::DescriptorPool::Create(
	    device, 1, {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}});
	{
		VkDescriptorSetLayoutBinding image_binding = {.binding = 0,
		                                              .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		                                              .descriptorCount = 1,
		                                              .stageFlags =
		                                                  VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT};
		VkDescriptorSetLayoutBinding prab_buffer_binding = {.binding = 1,
		                                                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		                                                    .descriptorCount = 1,
		                                                    .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT};
		VkDescriptorSetLayoutBinding alias_buffer_binding = {.binding = 2,
		                                                     .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		                                                     .descriptorCount = 1,
		                                                     .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT};

		m_descriptor_set_layout =
		    myvk::DescriptorSetLayout::Create(device, {image_binding, prab_buffer_binding, alias_buffer_binding});
	}
	m_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_descriptor_set_layout);
}
void EnvironmentMap::Reset() {}
void EnvironmentMap::Reset(const std::shared_ptr<myvk::CommandPool> &m_command_pool, const char *filename) {}

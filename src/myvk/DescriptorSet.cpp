#include "DescriptorSet.hpp"

namespace myvk {

std::shared_ptr<DescriptorSet>
DescriptorSet::Create(const std::shared_ptr<DescriptorPool> &descriptor_pool,
                      const std::shared_ptr<DescriptorSetLayout> &descriptor_set_layout) {
	std::shared_ptr<DescriptorSet> ret = std::make_shared<DescriptorSet>();
	ret->m_descriptor_pool_ptr = descriptor_pool;
	ret->m_descriptor_set_layout_ptr = descriptor_set_layout;

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = descriptor_pool->GetHandle();
	alloc_info.descriptorSetCount = 1;
	VkDescriptorSetLayout layout = descriptor_set_layout->GetHandle();
	alloc_info.pSetLayouts = &layout;

	if (vkAllocateDescriptorSets(descriptor_pool->GetDevicePtr()->GetHandle(), &alloc_info, &ret->m_descriptor_set) !=
	    VK_SUCCESS)
		return nullptr;
	return ret;
}

std::vector<std::shared_ptr<DescriptorSet>>
DescriptorSet::CreateMultiple(const std::shared_ptr<DescriptorPool> &descriptor_pool,
                              const std::vector<std::shared_ptr<DescriptorSetLayout>> &descriptor_set_layouts) {
	std::vector<std::shared_ptr<DescriptorSet>> ret(descriptor_set_layouts.size());
	for (uint32_t i = 0; i < ret.size(); ++i) {
		ret[i] = std::make_shared<DescriptorSet>();
		ret[i]->m_descriptor_pool_ptr = descriptor_pool;
		ret[i]->m_descriptor_set_layout_ptr = descriptor_set_layouts[i];
	}

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = descriptor_pool->GetHandle();
	alloc_info.descriptorSetCount = ret.size();
	std::vector<VkDescriptorSetLayout> layouts(ret.size());
	for (uint32_t i = 0; i < ret.size(); ++i)
		layouts[i] = descriptor_set_layouts[i]->GetHandle();
	alloc_info.pSetLayouts = layouts.data();

	std::vector<VkDescriptorSet> handles(ret.size());
	if (vkAllocateDescriptorSets(descriptor_pool->GetDevicePtr()->GetHandle(), &alloc_info, handles.data()) !=
	    VK_SUCCESS)
		return {};

	for (uint32_t i = 0; i < ret.size(); ++i)
		ret[i]->m_descriptor_set = handles[i];

	return ret;
}

void DescriptorSet::UpdateUniformBuffer(const std::shared_ptr<BufferBase> &buffer, uint32_t binding,
                                        uint32_t array_element, VkDeviceSize offset, VkDeviceSize range) const {
	VkDescriptorBufferInfo info = {};
	info.buffer = buffer->GetHandle();
	info.range = range;
	info.offset = offset;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = m_descriptor_set;
	write.dstBinding = binding;
	write.dstArrayElement = array_element;
	write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write.descriptorCount = 1;
	write.pBufferInfo = &info;

	vkUpdateDescriptorSets(GetDevicePtr()->GetHandle(), 1, &write, 0, nullptr);
}

void DescriptorSet::UpdateStorageBuffer(const std::shared_ptr<BufferBase> &buffer, uint32_t binding,
                                        uint32_t array_element, VkDeviceSize offset, VkDeviceSize range) const {
	VkDescriptorBufferInfo info = {};
	info.buffer = buffer->GetHandle();
	info.range = range;
	info.offset = offset;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = m_descriptor_set;
	write.dstBinding = binding;
	write.dstArrayElement = array_element;
	write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	write.descriptorCount = 1;
	write.pBufferInfo = &info;

	vkUpdateDescriptorSets(GetDevicePtr()->GetHandle(), 1, &write, 0, nullptr);
}

void DescriptorSet::UpdateCombinedImageSampler(const std::shared_ptr<Sampler> &sampler,
                                               const std::shared_ptr<ImageView> &image_view, uint32_t binding,
                                               uint32_t array_element) const {
	VkDescriptorImageInfo info = {};
	info.imageView = image_view->GetHandle();
	info.sampler = sampler->GetHandle();
	info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = m_descriptor_set;
	write.dstBinding = binding;
	write.dstArrayElement = array_element;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.descriptorCount = 1;
	write.pImageInfo = &info;

	vkUpdateDescriptorSets(GetDevicePtr()->GetHandle(), 1, &write, 0, nullptr);
}

void DescriptorSet::UpdateStorageImage(const std::shared_ptr<ImageView> &image_view, uint32_t binding,
                                       uint32_t array_element) const {
	VkDescriptorImageInfo info = {};
	info.imageView = image_view->GetHandle();
	info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = m_descriptor_set;
	write.dstBinding = binding;
	write.dstArrayElement = array_element;
	write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	write.descriptorCount = 1;
	write.pImageInfo = &info;

	vkUpdateDescriptorSets(GetDevicePtr()->GetHandle(), 1, &write, 0, nullptr);
}

DescriptorSet::~DescriptorSet() {
	if (m_descriptor_set)
		vkFreeDescriptorSets(m_descriptor_pool_ptr->GetDevicePtr()->GetHandle(), m_descriptor_pool_ptr->GetHandle(), 1,
		                     &m_descriptor_set);
}
} // namespace myvk

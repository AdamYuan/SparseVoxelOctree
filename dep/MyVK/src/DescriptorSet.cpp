#include "myvk/DescriptorSet.hpp"

namespace myvk {

Ptr<DescriptorSet> DescriptorSet::Create(const Ptr<DescriptorPool> &descriptor_pool,
                                         const Ptr<DescriptorSetLayout> &descriptor_set_layout) {
	auto ret = std::make_shared<DescriptorSet>();
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

std::vector<Ptr<DescriptorSet>>
DescriptorSet::CreateMultiple(const Ptr<DescriptorPool> &descriptor_pool,
                              const std::vector<Ptr<DescriptorSetLayout>> &descriptor_set_layouts) {
	uint32_t count = descriptor_set_layouts.size();

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = descriptor_pool->GetHandle();
	alloc_info.descriptorSetCount = count;
	std::vector<VkDescriptorSetLayout> layouts(count);
	for (uint32_t i = 0; i < count; ++i)
		layouts[i] = descriptor_set_layouts[i]->GetHandle();
	alloc_info.pSetLayouts = layouts.data();

	std::vector<VkDescriptorSet> handles(count);
	if (vkAllocateDescriptorSets(descriptor_pool->GetDevicePtr()->GetHandle(), &alloc_info, handles.data()) !=
	    VK_SUCCESS)
		return {};

	std::vector<Ptr<DescriptorSet>> ret(count);
	for (uint32_t i = 0; i < count; ++i) {
		auto ptr = std::make_shared<DescriptorSet>();
		ptr->m_descriptor_pool_ptr = descriptor_pool;
		ptr->m_descriptor_set_layout_ptr = descriptor_set_layouts[i];
		ptr->m_descriptor_set = handles[i];

		ret[i] = ptr;
	}
	return ret;
}

void DescriptorSet::UpdateUniformBuffer(const Ptr<BufferBase> &buffer, uint32_t binding, uint32_t array_element,
                                        VkDeviceSize offset, VkDeviceSize range) const {
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

void DescriptorSet::UpdateStorageBuffer(const Ptr<BufferBase> &buffer, uint32_t binding, uint32_t array_element,
                                        VkDeviceSize offset, VkDeviceSize range) const {
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

void DescriptorSet::UpdateCombinedImageSampler(const Ptr<Sampler> &sampler, const Ptr<ImageView> &image_view,
                                               uint32_t binding, uint32_t array_element) const {
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

void DescriptorSet::UpdateInputAttachment(const Ptr<ImageView> &image_view, uint32_t binding,
                                          uint32_t array_element) const {
	VkDescriptorImageInfo info = {};
	info.imageView = image_view->GetHandle();
	info.sampler = VK_NULL_HANDLE;
	info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
	write.dstSet = m_descriptor_set;
	write.dstBinding = binding;
	write.dstArrayElement = array_element;
	write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	write.descriptorCount = 1;
	write.pImageInfo = &info;

	vkUpdateDescriptorSets(GetDevicePtr()->GetHandle(), 1, &write, 0, nullptr);
}

void DescriptorSet::UpdateStorageImage(const Ptr<ImageView> &image_view, uint32_t binding,
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

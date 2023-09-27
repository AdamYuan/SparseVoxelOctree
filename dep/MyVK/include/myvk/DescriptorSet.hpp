#ifndef MYVK_DESCRIPTOR_SET_HPP
#define MYVK_DESCRIPTOR_SET_HPP

#include "BufferBase.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "DeviceObjectBase.hpp"
#include "ImageView.hpp"
#include "Sampler.hpp"

#include <memory>

namespace myvk {
class DescriptorSet : public DeviceObjectBase {
private:
	Ptr<DescriptorPool> m_descriptor_pool_ptr;
	Ptr<DescriptorSetLayout> m_descriptor_set_layout_ptr;

	VkDescriptorSet m_descriptor_set{VK_NULL_HANDLE};

public:
	static Ptr<DescriptorSet> Create(const Ptr<DescriptorPool> &descriptor_pool,
	                                 const Ptr<DescriptorSetLayout> &descriptor_set_layout);

	static std::vector<Ptr<DescriptorSet>>
	CreateMultiple(const Ptr<DescriptorPool> &descriptor_pool,
	               const std::vector<Ptr<DescriptorSetLayout>> &descriptor_set_layouts);

	void UpdateUniformBuffer(const Ptr<BufferBase> &buffer, uint32_t binding, uint32_t array_element = 0,
	                         VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE) const;

	void UpdateCombinedImageSampler(const Ptr<Sampler> &sampler, const Ptr<ImageView> &image_view, uint32_t binding,
	                                uint32_t array_element = 0) const;

	void UpdateStorageBuffer(const Ptr<BufferBase> &buffer, uint32_t binding, uint32_t array_element = 0,
	                         VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE) const;

	void UpdateStorageImage(const Ptr<ImageView> &image_view, uint32_t binding, uint32_t array_element = 0) const;
	void UpdateInputAttachment(const Ptr<ImageView> &image_view, uint32_t binding, uint32_t array_element = 0) const;

	VkDescriptorSet GetHandle() const { return m_descriptor_set; }

	const Ptr<Device> &GetDevicePtr() const override { return m_descriptor_pool_ptr->GetDevicePtr(); }

	const Ptr<DescriptorPool> &GetDescriptorPoolPtr() const { return m_descriptor_pool_ptr; }

	const Ptr<DescriptorSetLayout> &GetDescriptorSetLayoutPtr() const { return m_descriptor_set_layout_ptr; }

	~DescriptorSet() override;
};
} // namespace myvk

#endif

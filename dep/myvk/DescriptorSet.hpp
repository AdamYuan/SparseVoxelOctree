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
	std::shared_ptr<DescriptorPool> m_descriptor_pool_ptr;
	std::shared_ptr<DescriptorSetLayout> m_descriptor_set_layout_ptr;

	VkDescriptorSet m_descriptor_set{VK_NULL_HANDLE};

public:
	static std::shared_ptr<DescriptorSet> Create(const std::shared_ptr<DescriptorPool> &descriptor_pool,
	                                             const std::shared_ptr<DescriptorSetLayout> &descriptor_set_layout);

	static std::vector<std::shared_ptr<DescriptorSet>>
	CreateMultiple(const std::shared_ptr<DescriptorPool> &descriptor_pool,
	               const std::vector<std::shared_ptr<DescriptorSetLayout>> &descriptor_set_layouts);

	void UpdateUniformBuffer(const std::shared_ptr<BufferBase> &buffer, uint32_t binding, uint32_t array_element = 0,
	                         VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE) const;

	void UpdateCombinedImageSampler(const std::shared_ptr<Sampler> &sampler,
	                                const std::shared_ptr<ImageView> &image_view, uint32_t binding,
	                                uint32_t array_element = 0) const;

	void UpdateStorageBuffer(const std::shared_ptr<BufferBase> &buffer, uint32_t binding, uint32_t array_element = 0,
	                         VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE) const;

	void UpdateStorageImage(const std::shared_ptr<ImageView> &image_view, uint32_t binding,
	                        uint32_t array_element = 0) const;

	VkDescriptorSet GetHandle() const { return m_descriptor_set; }

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_descriptor_pool_ptr->GetDevicePtr(); }

	const std::shared_ptr<DescriptorPool> &GetDescriptorPoolPtr() const { return m_descriptor_pool_ptr; }

	const std::shared_ptr<DescriptorSetLayout> &GetDescriptorSetLayoutPtr() const {
		return m_descriptor_set_layout_ptr;
	}

	~DescriptorSet();
};
} // namespace myvk

#endif

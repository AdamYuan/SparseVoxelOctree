#ifndef MYVK_IMAGE_HPP
#define MYVK_IMAGE_HPP

#include "ImageBase.hpp"
#include "Queue.hpp"

namespace myvk {
class Image : public ImageBase {
private:
	Ptr<Device> m_device_ptr;

	VmaAllocation m_allocation{VK_NULL_HANDLE};

public:
	static Ptr<Image> Create(const Ptr<Device> &device, const VkImageCreateInfo &create_info,
	                         VmaAllocationCreateFlags allocation_flags = 0,
	                         VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
	                         const std::vector<Ptr<Queue>> &access_queues = {});

	static Ptr<Image> CreateTexture2D(const Ptr<Device> &device, const VkExtent2D &size, uint32_t mip_level,
	                                  VkFormat format, VkImageUsageFlags usage,
	                                  const std::vector<Ptr<Queue>> &access_queue = {});

	static Ptr<Image> CreateTexture2DArray(const Ptr<Device> &device, const VkExtent2D &size, uint32_t array_layer,
	                                       uint32_t mip_level, VkFormat format, VkImageUsageFlags usage,
	                                       const std::vector<Ptr<Queue>> &access_queue = {});

	static Ptr<Image> CreateTexture3D(const Ptr<Device> &device, const VkExtent3D &size, uint32_t mip_level,
	                                  VkFormat format, VkImageUsageFlags usage,
	                                  const std::vector<Ptr<Queue>> &access_queue = {});

	const Ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	~Image() override;
};
} // namespace myvk

#endif

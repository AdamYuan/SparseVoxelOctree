#ifndef MYVK_IMAGE_HPP
#define MYVK_IMAGE_HPP

#include "ImageBase.hpp"
#include "Queue.hpp"

namespace myvk {
class Image : public ImageBase {
private:
	std::shared_ptr<Device> m_device_ptr;

	VmaAllocation m_allocation{VK_NULL_HANDLE};

public:
	static std::shared_ptr<Image> Create(const std::shared_ptr<Device> &device, VmaMemoryUsage memory_usage,
	                                     const VkImageCreateInfo &create_info,
	                                     const std::vector<std::shared_ptr<Queue>> &access_queues = {});

	static uint32_t QueryMipLevel(uint32_t w);

	static uint32_t QueryMipLevel(const VkExtent2D &size);

	static uint32_t QueryMipLevel(const VkExtent3D &size);

	static std::shared_ptr<Image> CreateTexture2D(const std::shared_ptr<Device> &device, const VkExtent2D &size,
	                                              uint32_t mip_level, VkFormat format, VkImageUsageFlags usage,
	                                              const std::vector<std::shared_ptr<Queue>> &access_queue = {});

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	~Image();
};
} // namespace myvk

#endif

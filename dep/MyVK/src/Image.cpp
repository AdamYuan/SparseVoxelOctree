#include "myvk/Image.hpp"
#include <set>

namespace myvk {
Ptr<Image> Image::Create(const Ptr<Device> &device, const VkImageCreateInfo &create_info,
                         VmaAllocationCreateFlags allocation_flags, VmaMemoryUsage memory_usage,
                         const std::vector<Ptr<Queue>> &access_queues) {
	auto ret = std::make_shared<Image>();
	ret->m_device_ptr = device;

	ret->m_usage = create_info.usage;
	ret->m_extent = create_info.extent;
	ret->m_type = create_info.imageType;
	ret->m_format = create_info.format;
	ret->m_mip_levels = create_info.mipLevels;
	ret->m_array_layers = create_info.arrayLayers;

	std::set<uint32_t> queue_family_set;
	for (auto &i : access_queues)
		queue_family_set.insert(i->GetFamilyIndex());
	std::vector<uint32_t> queue_families{queue_family_set.begin(), queue_family_set.end()};

	VkImageCreateInfo new_info{create_info};
	if (queue_families.size() <= 1) {
		new_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	} else {
		new_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
		new_info.queueFamilyIndexCount = queue_families.size();
		new_info.pQueueFamilyIndices = queue_families.data();
	}

	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.flags = allocation_flags;
	alloc_info.usage = memory_usage;

	if (vmaCreateImage(device->GetAllocatorHandle(), &new_info, &alloc_info, &ret->m_image, &ret->m_allocation,
	                   nullptr) != VK_SUCCESS)
		return nullptr;
	return ret;
}

Ptr<Image> Image::CreateTexture2D(const Ptr<Device> &device, const VkExtent2D &size, uint32_t mip_level,
                                  VkFormat format, VkImageUsageFlags usage,
                                  const std::vector<Ptr<Queue>> &access_queue) {
	VkImageCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.extent = {size.width, size.height, 1};
	create_info.mipLevels = mip_level;
	create_info.arrayLayers = 1;
	create_info.format = format;
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	create_info.usage = usage;
	create_info.samples = VK_SAMPLE_COUNT_1_BIT;

	return Create(device, create_info, 0, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, access_queue);
}

Ptr<Image> Image::CreateTexture3D(const Ptr<Device> &device, const VkExtent3D &size, uint32_t mip_level,
                                  VkFormat format, VkImageUsageFlags usage,
                                  const std::vector<Ptr<Queue>> &access_queue) {
	VkImageCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.imageType = VK_IMAGE_TYPE_3D;
	create_info.extent = size;
	create_info.mipLevels = mip_level;
	create_info.arrayLayers = 1;
	create_info.format = format;
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	create_info.usage = usage;
	create_info.samples = VK_SAMPLE_COUNT_1_BIT;

	return Create(device, create_info, 0, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, access_queue);
}

Ptr<Image> Image::CreateTexture2DArray(const Ptr<Device> &device, const VkExtent2D &size, uint32_t array_layer,
                                       uint32_t mip_level, VkFormat format, VkImageUsageFlags usage,
                                       const std::vector<Ptr<Queue>> &access_queue) {
	VkImageCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.extent = {size.width, size.height, 1};
	create_info.mipLevels = mip_level;
	create_info.arrayLayers = array_layer;
	create_info.format = format;
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	create_info.usage = usage;
	create_info.samples = VK_SAMPLE_COUNT_1_BIT;

	return Create(device, create_info, 0, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, access_queue);
}

Image::~Image() {
	if (m_image)
		vmaDestroyImage(m_device_ptr->GetAllocatorHandle(), m_image, m_allocation);
}
} // namespace myvk

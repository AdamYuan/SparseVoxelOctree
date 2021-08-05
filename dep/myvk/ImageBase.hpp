#ifndef MYVK_IMAGE_BASE_HPP
#define MYVK_IMAGE_BASE_HPP

#include "DeviceObjectBase.hpp"
#include <volk.h>

namespace myvk {
class ImageBase : public DeviceObjectBase {
protected:
	VkImage m_image{VK_NULL_HANDLE};

	VkExtent3D m_extent{};
	VkImageType m_type{};
	VkFormat m_format{};
	uint32_t m_mip_levels{};
	uint32_t m_array_layers{};

public:
	VkImage GetHandle() const { return m_image; }

	const VkExtent3D &GetExtent() const { return m_extent; }

	VkImageType GetType() const { return m_type; }

	VkFormat GetFormat() const { return m_format; }

	uint32_t GetMipLevels() const { return m_mip_levels; }

	uint32_t GetArrayLayers() const { return m_array_layers; }

	VkImageSubresourceRange GetSubresourceRange(VkImageAspectFlags aspect_mask) const {
		return {aspect_mask, 0, m_mip_levels, 0, m_array_layers};
	}

	std::vector<VkImageMemoryBarrier> GetDstMemoryBarriers(const std::vector<VkBufferImageCopy> &regions,
	                                                       VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
	                                                       VkImageLayout old_layout, VkImageLayout new_layout,
	                                                       uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                                       uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;

	std::vector<VkImageMemoryBarrier> GetMemoryBarriers(const std::vector<VkImageSubresourceLayers> &regions,
	                                                    VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
	                                                    VkImageLayout old_layout, VkImageLayout new_layout,
	                                                    uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                                    uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;

	std::vector<VkImageMemoryBarrier> GetMemoryBarriers(const std::vector<VkImageSubresourceRange> &regions,
	                                                    VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
	                                                    VkImageLayout old_layout, VkImageLayout new_layout,
	                                                    uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                                    uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;

	VkImageMemoryBarrier GetMemoryBarrier(VkImageAspectFlags aspect_mask, VkAccessFlags src_access_mask,
	                                      VkAccessFlags dst_access_mask, VkImageLayout old_layout,
	                                      VkImageLayout new_layout, uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                      uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;

	VkImageMemoryBarrier GetDstMemoryBarrier(const VkBufferImageCopy &region, VkAccessFlags src_access_mask,
	                                         VkAccessFlags dst_access_mask, VkImageLayout old_layout,
	                                         VkImageLayout new_layout,
	                                         uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                         uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;

	VkImageMemoryBarrier GetMemoryBarrier(const VkImageSubresourceLayers &region, VkAccessFlags src_access_mask,
	                                      VkAccessFlags dst_access_mask, VkImageLayout old_layout,
	                                      VkImageLayout new_layout, uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                      uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;

	VkImageMemoryBarrier GetMemoryBarrier(const VkImageSubresourceRange &region, VkAccessFlags src_access_mask,
	                                      VkAccessFlags dst_access_mask, VkImageLayout old_layout,
	                                      VkImageLayout new_layout, uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                      uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;
};
} // namespace myvk

#endif

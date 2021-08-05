#include "ImageBase.hpp"
#include "Queue.hpp"

namespace myvk {
std::vector<VkImageMemoryBarrier>
ImageBase::GetDstMemoryBarriers(const std::vector<VkBufferImageCopy> &regions, VkAccessFlags src_access_mask,
                                VkAccessFlags dst_access_mask, VkImageLayout old_layout, VkImageLayout new_layout,
                                uint32_t src_queue_family, uint32_t dst_queue_family) const {
	std::vector<VkImageMemoryBarrier> barriers(regions.size());

	for (uint32_t i = 0; i < regions.size(); ++i) {
		VkImageMemoryBarrier &cur = barriers[i];
		cur.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		cur.image = m_image;
		cur.oldLayout = old_layout;
		cur.newLayout = new_layout;
		cur.srcAccessMask = src_access_mask;
		cur.dstAccessMask = dst_access_mask;
		cur.srcQueueFamilyIndex = src_queue_family;
		cur.dstQueueFamilyIndex = dst_queue_family;
		VkImageSubresourceRange &range = cur.subresourceRange;
		range.aspectMask = regions[i].imageSubresource.aspectMask;
		range.baseMipLevel = regions[i].imageSubresource.mipLevel;
		range.levelCount = 1;
		range.baseArrayLayer = regions[i].imageSubresource.baseArrayLayer;
		range.layerCount = regions[i].imageSubresource.layerCount;
	}
	return barriers;
}

std::vector<VkImageMemoryBarrier> ImageBase::GetMemoryBarriers(const std::vector<VkImageSubresourceLayers> &regions,
                                                               VkAccessFlags src_access_mask,
                                                               VkAccessFlags dst_access_mask, VkImageLayout old_layout,
                                                               VkImageLayout new_layout, uint32_t src_queue_family,
                                                               uint32_t dst_queue_family) const {
	std::vector<VkImageMemoryBarrier> barriers(regions.size());
	for (uint32_t i = 0; i < regions.size(); ++i) {
		VkImageMemoryBarrier &cur = barriers[i];
		cur.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		cur.image = m_image;
		cur.oldLayout = old_layout;
		cur.newLayout = new_layout;
		cur.srcAccessMask = src_access_mask;
		cur.dstAccessMask = dst_access_mask;
		cur.srcQueueFamilyIndex = src_queue_family;
		cur.dstQueueFamilyIndex = dst_queue_family;
		cur.subresourceRange.aspectMask = regions[i].aspectMask;
		cur.subresourceRange.baseMipLevel = regions[i].mipLevel;
		cur.subresourceRange.baseArrayLayer = regions[i].baseArrayLayer;
		cur.subresourceRange.levelCount = 1;
		cur.subresourceRange.layerCount = regions[i].layerCount;
	}
	return barriers;
}

std::vector<VkImageMemoryBarrier> ImageBase::GetMemoryBarriers(const std::vector<VkImageSubresourceRange> &regions,
                                                               VkAccessFlags src_access_mask,
                                                               VkAccessFlags dst_access_mask, VkImageLayout old_layout,
                                                               VkImageLayout new_layout, uint32_t src_queue_family,
                                                               uint32_t dst_queue_family) const {
	std::vector<VkImageMemoryBarrier> barriers(regions.size());
	for (uint32_t i = 0; i < regions.size(); ++i) {
		VkImageMemoryBarrier &cur = barriers[i];
		cur.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		cur.image = m_image;
		cur.oldLayout = old_layout;
		cur.newLayout = new_layout;
		cur.srcAccessMask = src_access_mask;
		cur.dstAccessMask = dst_access_mask;
		cur.srcQueueFamilyIndex = src_queue_family;
		cur.dstQueueFamilyIndex = dst_queue_family;
		cur.subresourceRange = regions[i];
	}
	return barriers;
}

VkImageMemoryBarrier ImageBase::GetMemoryBarrier(VkImageAspectFlags aspect_mask, VkAccessFlags src_access_mask,
                                                 VkAccessFlags dst_access_mask, VkImageLayout old_layout,
                                                 VkImageLayout new_layout, uint32_t src_queue_family,
                                                 uint32_t dst_queue_family) const {
	VkImageMemoryBarrier ret = {};
	ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	ret.image = m_image;
	ret.oldLayout = old_layout;
	ret.newLayout = new_layout;
	ret.srcAccessMask = src_access_mask;
	ret.dstAccessMask = dst_access_mask;
	ret.srcQueueFamilyIndex = src_queue_family;
	ret.dstQueueFamilyIndex = dst_queue_family;
	ret.subresourceRange = GetSubresourceRange(aspect_mask);
	return ret;
}

VkImageMemoryBarrier ImageBase::GetDstMemoryBarrier(const VkBufferImageCopy &region, VkAccessFlags src_access_mask,
                                                    VkAccessFlags dst_access_mask, VkImageLayout old_layout,
                                                    VkImageLayout new_layout, uint32_t src_queue_family,
                                                    uint32_t dst_queue_family) const {
	VkImageMemoryBarrier ret = {};
	ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	ret.image = m_image;
	ret.oldLayout = old_layout;
	ret.newLayout = new_layout;
	ret.srcAccessMask = src_access_mask;
	ret.dstAccessMask = dst_access_mask;
	ret.srcQueueFamilyIndex = src_queue_family;
	ret.dstQueueFamilyIndex = dst_queue_family;
	VkImageSubresourceRange &range = ret.subresourceRange;
	range.aspectMask = region.imageSubresource.aspectMask;
	range.baseMipLevel = region.imageSubresource.mipLevel;
	range.levelCount = 1;
	range.baseArrayLayer = region.imageSubresource.baseArrayLayer;
	range.layerCount = region.imageSubresource.layerCount;
	return ret;
}

VkImageMemoryBarrier ImageBase::GetMemoryBarrier(const VkImageSubresourceLayers &region, VkAccessFlags src_access_mask,
                                                 VkAccessFlags dst_access_mask, VkImageLayout old_layout,
                                                 VkImageLayout new_layout, uint32_t src_queue_family,
                                                 uint32_t dst_queue_family) const {
	VkImageMemoryBarrier ret = {};
	ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	ret.image = m_image;
	ret.oldLayout = old_layout;
	ret.newLayout = new_layout;
	ret.srcAccessMask = src_access_mask;
	ret.dstAccessMask = dst_access_mask;
	ret.srcQueueFamilyIndex = src_queue_family;
	ret.dstQueueFamilyIndex = dst_queue_family;
	VkImageSubresourceRange &range = ret.subresourceRange;
	range.aspectMask = region.aspectMask;
	range.baseMipLevel = region.mipLevel;
	range.levelCount = 1;
	range.baseArrayLayer = region.baseArrayLayer;
	range.layerCount = region.layerCount;
	return ret;
}

VkImageMemoryBarrier ImageBase::GetMemoryBarrier(const VkImageSubresourceRange &region, VkAccessFlags src_access_mask,
                                                 VkAccessFlags dst_access_mask, VkImageLayout old_layout,
                                                 VkImageLayout new_layout, uint32_t src_queue_family,
                                                 uint32_t dst_queue_family) const {
	VkImageMemoryBarrier ret = {};
	ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	ret.image = m_image;
	ret.oldLayout = old_layout;
	ret.newLayout = new_layout;
	ret.srcAccessMask = src_access_mask;
	ret.dstAccessMask = dst_access_mask;
	ret.srcQueueFamilyIndex = src_queue_family;
	ret.dstQueueFamilyIndex = dst_queue_family;
	ret.subresourceRange = region;
	return ret;
}
} // namespace myvk

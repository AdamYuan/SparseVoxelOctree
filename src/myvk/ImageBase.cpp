#include "ImageBase.hpp"

namespace myvk {
	std::vector<VkImageMemoryBarrier>
	ImageBase::GetDstMemoryBarriers(const std::vector<VkBufferImageCopy> &regions, VkAccessFlags src_access_mask,
									VkAccessFlags dst_access_mask, VkImageLayout old_layout,
									VkImageLayout new_layout) const {
		std::vector<VkImageMemoryBarrier> barriers(regions.size());

		for (uint32_t i = 0; i < regions.size(); ++i) {
			VkImageMemoryBarrier &cur = barriers[i];
			cur.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			cur.image = m_image;
			cur.oldLayout = old_layout;
			cur.newLayout = new_layout;
			cur.srcAccessMask = src_access_mask;
			cur.dstAccessMask = dst_access_mask;
			cur.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			cur.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			VkImageSubresourceRange &range = cur.subresourceRange;
			range.aspectMask = regions[i].imageSubresource.aspectMask;
			range.baseMipLevel = regions[i].imageSubresource.mipLevel;
			range.levelCount = 1;
			range.baseArrayLayer = regions[i].imageSubresource.baseArrayLayer;
			range.layerCount = regions[i].imageSubresource.layerCount;
		}
		return barriers;
	}

	std::vector<VkImageMemoryBarrier>
	ImageBase::GetMemoryBarriers(const std::vector<VkImageSubresourceLayers> &regions, VkAccessFlags src_access_mask,
								 VkAccessFlags dst_access_mask, VkImageLayout old_layout,
								 VkImageLayout new_layout) const {
		std::vector<VkImageMemoryBarrier> barriers(regions.size());
		for (uint32_t i = 0; i < regions.size(); ++i) {
			VkImageMemoryBarrier &cur = barriers[i];
			cur.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			cur.image = m_image;
			cur.oldLayout = old_layout;
			cur.newLayout = new_layout;
			cur.srcAccessMask = src_access_mask;
			cur.dstAccessMask = dst_access_mask;
			cur.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			cur.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			cur.subresourceRange.aspectMask = regions[i].aspectMask;
			cur.subresourceRange.baseMipLevel = regions[i].mipLevel;
			cur.subresourceRange.baseArrayLayer = regions[i].baseArrayLayer;
			cur.subresourceRange.levelCount = 1;
			cur.subresourceRange.layerCount = regions[i].layerCount;
		}
		return barriers;
	}

	std::vector<VkImageMemoryBarrier>
	ImageBase::GetMemoryBarriers(const std::vector<VkImageSubresourceRange> &regions, VkAccessFlags src_access_mask,
								 VkAccessFlags dst_access_mask, VkImageLayout old_layout,
								 VkImageLayout new_layout) const {
		std::vector<VkImageMemoryBarrier> barriers(regions.size());
		for (uint32_t i = 0; i < regions.size(); ++i) {
			VkImageMemoryBarrier &cur = barriers[i];
			cur.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			cur.image = m_image;
			cur.oldLayout = old_layout;
			cur.newLayout = new_layout;
			cur.srcAccessMask = src_access_mask;
			cur.dstAccessMask = dst_access_mask;
			cur.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			cur.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			cur.subresourceRange = regions[i];
		}
		return barriers;
	}
}

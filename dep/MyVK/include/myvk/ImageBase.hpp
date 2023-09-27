#ifndef MYVK_IMAGE_BASE_HPP
#define MYVK_IMAGE_BASE_HPP

#include "DeviceObjectBase.hpp"
#include "volk.h"

namespace myvk {
class ImageBase : public DeviceObjectBase {
private:
	static inline uint32_t simple_ctz(uint32_t x) {
		if (x & 0x80000000u)
			return 32u;
		uint32_t ret{1};
		while (x >> ret)
			++ret;
		return ret;
	}

protected:
	VkImage m_image{VK_NULL_HANDLE};

	// TODO: VkImageCreateFlags ?
	VkImageUsageFlags m_usage{};
	VkExtent3D m_extent{};
	VkImageType m_type{};
	VkFormat m_format{};
	uint32_t m_mip_levels{};
	uint32_t m_array_layers{};

public:
	~ImageBase() override = default;

	VkImage GetHandle() const { return m_image; }

	const VkExtent3D &GetExtent() const { return m_extent; }
	VkExtent2D GetExtent2D() const { return {m_extent.width, m_extent.height}; }

	VkImageUsageFlags GetUsage() const { return m_usage; }

	VkImageType GetType() const { return m_type; }

	VkFormat GetFormat() const { return m_format; }

	uint32_t GetMipLevels() const { return m_mip_levels; }

	uint32_t GetArrayLayers() const { return m_array_layers; }

	inline static uint32_t QueryMipLevel(uint32_t w) { return simple_ctz(w); }
	inline static uint32_t QueryMipLevel(const VkExtent2D &size) { return simple_ctz(size.width | size.height); }
	inline static uint32_t QueryMipLevel(const VkExtent3D &size) {
		return simple_ctz(size.width | size.height | size.depth);
	}

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

	inline VkFramebufferAttachmentImageInfo GetFramebufferAttachmentImageInfo() const {
		VkFramebufferAttachmentImageInfo ret = {VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO};
		ret.usage = m_usage;
		ret.width = m_extent.width;
		ret.height = m_extent.height;
		ret.layerCount = m_array_layers;
		ret.viewFormatCount = 1;
		ret.pViewFormats = &m_format;
		return ret;
	}
};
} // namespace myvk

#endif

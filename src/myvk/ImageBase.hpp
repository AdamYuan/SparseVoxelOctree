#ifndef MYVK_IMAGE_BASE_HPP
#define MYVK_IMAGE_BASE_HPP

#include <volk.h>
#include "DeviceObjectBase.hpp"

namespace myvk {
	class ImageBase : public DeviceObjectBase {
	protected:
		VkImage m_image{nullptr};

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

		std::vector<VkImageMemoryBarrier>
		GetDstMemoryBarriers(const std::vector<VkBufferImageCopy> &regions, VkAccessFlags src_access_mask,
							 VkAccessFlags dst_access_mask, VkImageLayout old_layout, VkImageLayout new_layout) const;

		std::vector<VkImageMemoryBarrier>
		GetMemoryBarriers(const std::vector<VkImageSubresourceLayers> &regions,
						  VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask, VkImageLayout old_layout,
						  VkImageLayout new_layout) const;

		std::vector<VkImageMemoryBarrier>
		GetMemoryBarriers(const std::vector<VkImageSubresourceRange> &regions,
						  VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask, VkImageLayout old_layout,
						  VkImageLayout new_layout) const;
	};
}

#endif

#ifndef MYVK_IMAGE_VIEW_HPP
#define MYVK_IMAGE_VIEW_HPP

#include "DeviceObjectBase.hpp"
#include "ImageBase.hpp"
#include "SwapchainImage.hpp"
#include "volk.h"
#include <memory>

namespace myvk {
class ImageView : public DeviceObjectBase {
private:
	Ptr<ImageBase> m_image_ptr;

	VkImageSubresourceRange m_subresource_range;
	VkImageView m_image_view{VK_NULL_HANDLE};

public:
	static Ptr<ImageView> Create(const Ptr<ImageBase> &image, const VkImageViewCreateInfo &create_info);

	static Ptr<ImageView> Create(const Ptr<ImageBase> &image, VkImageViewType view_type, VkFormat format,
	                             VkImageAspectFlags aspect_mask, uint32_t base_mip_level = 0, uint32_t level_count = 1,
	                             uint32_t base_array_layer = 0, uint32_t layer_count = 1,
	                             const VkComponentMapping &components = {
	                                 VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
	                                 VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY});

	static Ptr<ImageView> Create(const Ptr<ImageBase> &image, VkImageViewType view_type,
	                             VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT);

	static Ptr<ImageView> Create(const Ptr<ImageBase> &image, uint32_t mip_level, VkImageViewType view_type,
	                             VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT);

#ifdef MYVK_ENABLE_GLFW
	static Ptr<ImageView> Create(const Ptr<SwapchainImage> &swapchain_image);
#endif

	const Ptr<Device> &GetDevicePtr() const override { return m_image_ptr->GetDevicePtr(); }

	const Ptr<ImageBase> &GetImagePtr() const { return m_image_ptr; }

	const VkImageSubresourceRange &GetSubresourceRange() const { return m_subresource_range; }
	VkImageView GetHandle() const { return m_image_view; }

	VkImageMemoryBarrier GetMemoryBarrier(VkImageAspectFlags aspect_mask, VkAccessFlags src_access_mask,
	                                      VkAccessFlags dst_access_mask, VkImageLayout old_layout,
	                                      VkImageLayout new_layout, uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                      uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const {
		return m_image_ptr->GetMemoryBarrier(m_subresource_range, src_access_mask, dst_access_mask, old_layout,
		                                     new_layout, src_queue_family, dst_queue_family);
	}

	inline VkFramebufferAttachmentImageInfo GetFramebufferAttachmentImageInfo() const {
		VkFramebufferAttachmentImageInfo ret{m_image_ptr->GetFramebufferAttachmentImageInfo()};
		ret.layerCount = m_subresource_range.layerCount;
		return ret;
	}

	~ImageView() override;
};
} // namespace myvk

#endif

#include "ImageView.hpp"

namespace myvk {
std::shared_ptr<ImageView> ImageView::Create(const std::shared_ptr<ImageBase> &image, VkImageViewType view_type,
                                             VkFormat format, VkImageAspectFlags aspect_mask, uint32_t base_mip_level,
                                             uint32_t level_count, uint32_t base_array_layer, uint32_t layer_count,
                                             const VkComponentMapping &components) {
	VkImageViewCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.image = image->GetHandle();
	create_info.viewType = view_type;
	create_info.format = format;
	create_info.subresourceRange.aspectMask = aspect_mask;
	create_info.subresourceRange.baseMipLevel = base_mip_level;
	create_info.subresourceRange.levelCount = level_count;
	create_info.subresourceRange.baseArrayLayer = base_array_layer;
	create_info.subresourceRange.layerCount = layer_count;
	create_info.components = components;

	return Create(image, create_info);
}

std::shared_ptr<ImageView> ImageView::Create(const std::shared_ptr<SwapchainImage> &swapchain_image) {
	return Create(swapchain_image, VK_IMAGE_VIEW_TYPE_2D, swapchain_image->GetSwapchainPtr()->GetImageFormat(),
	              VK_IMAGE_ASPECT_COLOR_BIT);
}

std::shared_ptr<ImageView> ImageView::Create(const std::shared_ptr<ImageBase> &image, VkImageViewType view_type,
                                             VkImageAspectFlags aspect_mask) {
	return Create(image, view_type, image->GetFormat(), aspect_mask, 0, image->GetMipLevels(), 0,
	              image->GetArrayLayers());
}

std::shared_ptr<ImageView> ImageView::Create(const std::shared_ptr<ImageBase> &image,
                                             const VkImageViewCreateInfo &create_info) {
	std::shared_ptr<ImageView> ret = std::make_shared<ImageView>();
	ret->m_image_ptr = image;

	VkImageViewCreateInfo new_info = create_info;
	new_info.image = image->GetHandle();

	if (vkCreateImageView(image->GetDevicePtr()->GetHandle(), &new_info, nullptr, &ret->m_image_view) != VK_SUCCESS)
		return nullptr;

	return ret;
}

ImageView::~ImageView() {
	if (m_image_view)
		vkDestroyImageView(m_image_ptr->GetDevicePtr()->GetHandle(), m_image_view, nullptr);
}
} // namespace myvk

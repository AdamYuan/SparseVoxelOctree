#include "myvk/ImageView.hpp"

namespace myvk {
Ptr<ImageView> ImageView::Create(const Ptr<ImageBase> &image, VkImageViewType view_type, VkFormat format,
                                 VkImageAspectFlags aspect_mask, uint32_t base_mip_level, uint32_t level_count,
                                 uint32_t base_array_layer, uint32_t layer_count,
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

Ptr<ImageView> ImageView::Create(const Ptr<ImageBase> &image, VkImageViewType view_type,
                                 VkImageAspectFlags aspect_mask) {
	return Create(image, view_type, image->GetFormat(), aspect_mask, 0, image->GetMipLevels(), 0,
	              image->GetArrayLayers());
}

Ptr<ImageView> ImageView::Create(const Ptr<ImageBase> &image, uint32_t mip_level, VkImageViewType view_type,
                                 VkImageAspectFlags aspect_mask) {
	return Create(image, view_type, image->GetFormat(), aspect_mask, mip_level, 1, 0, image->GetArrayLayers());
}

Ptr<ImageView> ImageView::Create(const Ptr<ImageBase> &image, const VkImageViewCreateInfo &create_info) {
	auto ret = std::make_shared<ImageView>();
	ret->m_image_ptr = image;
	ret->m_subresource_range = create_info.subresourceRange;

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

#include "myvk/SwapchainImage.hpp"

namespace myvk {
std::vector<Ptr<SwapchainImage>> SwapchainImage::Create(const Ptr<Swapchain> &swapchain) {
	uint32_t image_count;
	vkGetSwapchainImagesKHR(swapchain->GetDevicePtr()->GetHandle(), swapchain->GetHandle(), &image_count, nullptr);
	std::vector<VkImage> images(image_count);
	vkGetSwapchainImagesKHR(swapchain->GetDevicePtr()->GetHandle(), swapchain->GetHandle(), &image_count,
	                        images.data());

	std::vector<Ptr<SwapchainImage>> ret(image_count);
	for (uint32_t i = 0; i < image_count; ++i) {
		auto ptr = std::make_shared<SwapchainImage>();
		ptr->m_swapchain_ptr = swapchain;
		ptr->m_image = images[i];

		ptr->m_usage = swapchain->GetImageUsage();
		ptr->m_extent = {swapchain->GetExtent().width, swapchain->GetExtent().height};
		ptr->m_type = VK_IMAGE_TYPE_2D;
		ptr->m_format = swapchain->GetImageFormat();
		ptr->m_mip_levels = 1;
		ptr->m_array_layers = 1;
		ret[i] = ptr;
	}
	return ret;
}
} // namespace myvk

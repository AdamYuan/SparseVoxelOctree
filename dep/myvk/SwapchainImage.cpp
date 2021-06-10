#include "SwapchainImage.hpp"

namespace myvk {
std::vector<std::shared_ptr<SwapchainImage>> SwapchainImage::Create(const std::shared_ptr<Swapchain> &swapchain) {
	uint32_t image_count;
	vkGetSwapchainImagesKHR(swapchain->GetDevicePtr()->GetHandle(), swapchain->GetHandle(), &image_count, nullptr);
	std::vector<VkImage> images(image_count);
	vkGetSwapchainImagesKHR(swapchain->GetDevicePtr()->GetHandle(), swapchain->GetHandle(), &image_count,
	                        images.data());

	std::vector<std::shared_ptr<SwapchainImage>> ret(image_count);
	for (uint32_t i = 0; i < image_count; ++i) {
		ret[i] = std::make_shared<SwapchainImage>();
		ret[i]->m_swapchain_ptr = swapchain;
		ret[i]->m_image = images[i];

		ret[i]->m_extent = {swapchain->GetExtent().width, swapchain->GetExtent().height};
		ret[i]->m_type = VK_IMAGE_TYPE_2D;
		ret[i]->m_format = swapchain->GetImageFormat();
		ret[i]->m_mip_levels = 1;
		ret[i]->m_array_layers = 1;
	}
	return ret;
}
} // namespace myvk

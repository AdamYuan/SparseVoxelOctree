#include "myvk/ImageView.hpp"

namespace myvk {
Ptr<ImageView> ImageView::Create(const Ptr<SwapchainImage> &swapchain_image) {
	return Create(swapchain_image, VK_IMAGE_VIEW_TYPE_2D, swapchain_image->GetSwapchainPtr()->GetImageFormat(),
	              VK_IMAGE_ASPECT_COLOR_BIT);
}
} // namespace myvk

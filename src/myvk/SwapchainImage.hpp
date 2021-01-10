#ifndef MYVK_SWAPCHAIN_IMAGE_HPP
#define MYVK_SWAPCHAIN_IMAGE_HPP

#include "ImageBase.hpp"
#include "Swapchain.hpp"

#include <volk.h>

#include <memory>
#include <vector>

namespace myvk {
class SwapchainImage : public ImageBase {
private:
	std::shared_ptr<Swapchain> m_swapchain_ptr;

public:
	static std::vector<std::shared_ptr<SwapchainImage>> Create(const std::shared_ptr<Swapchain> &swapchain);

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_swapchain_ptr->GetDevicePtr(); };
	const std::shared_ptr<Swapchain> &GetSwapchainPtr() const { return m_swapchain_ptr; }
};
} // namespace myvk

#endif

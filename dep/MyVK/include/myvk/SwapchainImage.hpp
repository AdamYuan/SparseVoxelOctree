#ifdef MYVK_ENABLE_GLFW

#ifndef MYVK_SWAPCHAIN_IMAGE_HPP
#define MYVK_SWAPCHAIN_IMAGE_HPP

#include "ImageBase.hpp"
#include "Swapchain.hpp"

#include "volk.h"

#include <memory>
#include <vector>

namespace myvk {
class SwapchainImage : public ImageBase {
private:
	Ptr<Swapchain> m_swapchain_ptr;

public:
	static std::vector<Ptr<SwapchainImage>> Create(const Ptr<Swapchain> &swapchain);
	~SwapchainImage() override = default;

	const Ptr<Device> &GetDevicePtr() const override { return m_swapchain_ptr->GetDevicePtr(); };
	const Ptr<Swapchain> &GetSwapchainPtr() const { return m_swapchain_ptr; }
};
} // namespace myvk

#endif

#endif

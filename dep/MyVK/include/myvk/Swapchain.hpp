#ifdef MYVK_ENABLE_GLFW

#ifndef MYVK_SWAPCHAIN_HPP
#define MYVK_SWAPCHAIN_HPP

#include "DeviceObjectBase.hpp"
#include "Fence.hpp"
#include "Queue.hpp"
#include "Semaphore.hpp"
#include "Surface.hpp"

#include "volk.h"
#include <cinttypes>
#include <memory>
#include <vector>

namespace myvk {
class Swapchain : public DeviceObjectBase {
private:
	Ptr<Queue> m_graphics_queue_ptr;
	Ptr<PresentQueue> m_present_queue_ptr;

	VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};

	VkSwapchainCreateInfoKHR m_swapchain_create_info;
	uint32_t m_image_count;

public:
	static Ptr<Swapchain> Create(const Ptr<Queue> &graphics_queue, const Ptr<PresentQueue> &present_queue,
	                             bool use_vsync, VkImageUsageFlags image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	static Ptr<Swapchain> Create(const Ptr<Swapchain> &old_swapchain);

	VkResult AcquireNextImage(uint32_t *p_image_index, const Ptr<Semaphore> &signal_semaphore,
	                          const Ptr<Fence> &signal_fence) const;

	VkResult Present(uint32_t image_index, const SemaphoreGroup &wait_semaphores) const;

	uint32_t GetImageCount() const { return m_image_count; }

	VkImageUsageFlags GetImageUsage() const { return m_swapchain_create_info.imageUsage; }

	VkSwapchainKHR GetHandle() const { return m_swapchain; };

	const Ptr<Queue> &GetGraphicsQueuePtr() const { return m_graphics_queue_ptr; }

	const Ptr<PresentQueue> &GetPresentQueuePtr() const { return m_present_queue_ptr; }

	const Ptr<Device> &GetDevicePtr() const override { return m_graphics_queue_ptr->GetDevicePtr(); };

	VkFormat GetImageFormat() const { return m_swapchain_create_info.imageFormat; }

	VkColorSpaceKHR GetImageColorSpace() const { return m_swapchain_create_info.imageColorSpace; }

	VkExtent2D GetExtent() const { return m_swapchain_create_info.imageExtent; }

	~Swapchain() override;
};
} // namespace myvk

#endif

#endif

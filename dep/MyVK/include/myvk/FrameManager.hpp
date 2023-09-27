#ifdef MYVK_ENABLE_GLFW

#ifndef MYVK_FRAME_MANAGER_HPP
#define MYVK_FRAME_MANAGER_HPP

#include "CommandBuffer.hpp"
#include "Fence.hpp"
#include "ImageView.hpp"
#include "Semaphore.hpp"
#include "Swapchain.hpp"
#include "SwapchainImage.hpp"

#include <functional>

namespace myvk {
class FrameManager : public DeviceObjectBase {
public:
	using ResizeFunc = std::function<void(const VkExtent2D &)>;

private:
	bool m_resized{false};
	uint32_t m_current_frame{0}, m_current_image_index, m_frame_count;
	ResizeFunc m_resize_func;

	Ptr<Swapchain> m_swapchain;
	std::vector<Ptr<SwapchainImage>> m_swapchain_images;
	std::vector<Ptr<ImageView>> m_swapchain_image_views;
	std::vector<const Fence *> m_image_fences;

	std::vector<Ptr<Fence>> m_frame_fences;
	std::vector<Ptr<Semaphore>> m_render_done_semaphores, m_acquire_done_semaphores;
	std::vector<Ptr<myvk::CommandBuffer>> m_frame_command_buffers;

	void initialize(const Ptr<Queue> &graphics_queue, const Ptr<PresentQueue> &present_queue, bool use_vsync,
	                uint32_t frame_count, VkImageUsageFlags image_usage);
	void recreate_swapchain();

public:
	static Ptr<FrameManager> Create(const Ptr<Queue> &graphics_queue, const Ptr<PresentQueue> &present_queue,
	                                bool use_vsync, uint32_t frame_count = 3,
	                                VkImageUsageFlags image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	~FrameManager() override = default;

	inline const Ptr<myvk::Device> &GetDevicePtr() const override { return m_swapchain->GetDevicePtr(); }

	void SetResizeFunc(const ResizeFunc &resize_func) { m_resize_func = resize_func; }
	void Resize() { m_resized = true; }

	bool NewFrame();
	void Render();

	void WaitIdle() const;

	uint32_t GetCurrentFrame() const { return m_current_frame; }
	uint32_t GetCurrentImageIndex() const { return m_current_image_index; }
	const Ptr<CommandBuffer> &GetCurrentCommandBuffer() const { return m_frame_command_buffers[m_current_frame]; }

	const Ptr<Swapchain> &GetSwapchain() const { return m_swapchain; }
	const std::vector<Ptr<SwapchainImage>> &GetSwapchainImages() const { return m_swapchain_images; }
	const std::vector<Ptr<ImageView>> &GetSwapchainImageViews() const { return m_swapchain_image_views; }

	const Ptr<SwapchainImage> &GetCurrentSwapchainImage() const { return m_swapchain_images[m_current_image_index]; }
	const Ptr<ImageView> &GetCurrentSwapchainImageView() const {
		return m_swapchain_image_views[m_current_image_index];
	}

	VkExtent2D GetExtent() const { return m_swapchain->GetExtent(); }

	void CmdPipelineSetScreenSize(const Ptr<myvk::CommandBuffer> &command_buffer) const;
};
} // namespace myvk

#endif

#endif

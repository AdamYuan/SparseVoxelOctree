#include "myvk/FrameManager.hpp"

namespace myvk {

void FrameManager::recreate_swapchain() {
	GLFWwindow *window = m_swapchain->GetPresentQueuePtr()->GetSurfacePtr()->GetGlfwWindow();
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	for (const auto &i : m_frame_fences)
		i->Wait();

	m_swapchain = Swapchain::Create(m_swapchain);
	m_swapchain_images = myvk::SwapchainImage::Create(m_swapchain);
	m_swapchain_image_views.clear();
	m_swapchain_image_views.resize(m_swapchain->GetImageCount());
	for (uint32_t i = 0; i < m_swapchain->GetImageCount(); ++i)
		m_swapchain_image_views[i] = myvk::ImageView::Create(m_swapchain_images[i]);
}

void FrameManager::initialize(const Ptr<Queue> &graphics_queue, const Ptr<PresentQueue> &present_queue, bool use_vsync,
                              uint32_t frame_count, VkImageUsageFlags image_usage) {
	m_swapchain = myvk::Swapchain::Create(graphics_queue, present_queue, use_vsync, image_usage);
	m_swapchain_images = myvk::SwapchainImage::Create(m_swapchain);
	m_swapchain_image_views.resize(m_swapchain->GetImageCount());
	for (uint32_t i = 0; i < m_swapchain->GetImageCount(); ++i)
		m_swapchain_image_views[i] = myvk::ImageView::Create(m_swapchain_images[i]);

	m_frame_count = frame_count;
	m_image_fences.resize(m_swapchain->GetImageCount(), nullptr);

	m_frame_fences.resize(frame_count);
	m_render_done_semaphores.resize(frame_count);
	m_acquire_done_semaphores.resize(frame_count);
	m_frame_command_buffers.resize(frame_count);

	for (uint32_t i = 0; i < frame_count; ++i) {
		m_frame_fences[i] = Fence::Create(m_swapchain->GetDevicePtr(), VK_FENCE_CREATE_SIGNALED_BIT);
		m_render_done_semaphores[i] = Semaphore::Create(m_swapchain->GetDevicePtr());
		m_acquire_done_semaphores[i] = Semaphore::Create(m_swapchain->GetDevicePtr());
		m_frame_command_buffers[i] = myvk::CommandBuffer::Create(myvk::CommandPool::Create(graphics_queue));
	}
}

bool FrameManager::NewFrame() {
	m_frame_fences[m_current_frame]->Wait();

	VkResult result =
	    m_swapchain->AcquireNextImage(&m_current_image_index, m_acquire_done_semaphores[m_current_frame], nullptr);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate_swapchain();
		m_resize_func(GetExtent());
		return false;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		// throw std::runtime_error("failed to acquire swap chain image!");
		return false;
	}

	if (m_image_fences[m_current_image_index])
		m_image_fences[m_current_image_index]->Wait();
	m_image_fences[m_current_image_index] = m_frame_fences[m_current_frame].get();

	// reset frame command buffer
	m_frame_command_buffers[m_current_frame]->GetCommandPoolPtr()->Reset();

	return true;
}

void FrameManager::Render() {
	m_frame_fences[m_current_frame]->Reset();
	m_frame_command_buffers[m_current_frame]->Submit(
	    {{m_acquire_done_semaphores[m_current_frame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}},
	    {m_render_done_semaphores[m_current_frame]}, m_frame_fences[m_current_frame]);
	VkResult result = m_swapchain->Present(m_current_image_index, {m_render_done_semaphores[m_current_frame]});
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_resized) {
		m_resized = false;
		recreate_swapchain();
		if (m_resize_func)
			m_resize_func(GetExtent());
	}

	m_current_frame = (m_current_frame + 1u) % m_frame_count;
}

void FrameManager::WaitIdle() const {
	for (const auto &i : m_frame_fences)
		i->Wait();
}
Ptr<FrameManager> FrameManager::Create(const Ptr<Queue> &graphics_queue, const Ptr<PresentQueue> &present_queue,
                                       bool use_vsync, uint32_t frame_count, VkImageUsageFlags image_usage) {
	auto ret = std::make_shared<FrameManager>();
	ret->initialize(graphics_queue, present_queue, use_vsync, frame_count, image_usage);
	return ret;
}
void FrameManager::CmdPipelineSetScreenSize(const Ptr<myvk::CommandBuffer> &command_buffer) const {
	VkExtent2D extent = m_swapchain->GetExtent();
	VkRect2D scissor = {};
	scissor.extent = extent;
	command_buffer->CmdSetScissor({scissor});
	VkViewport viewport = {};
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.maxDepth = 1.0f;
	command_buffer->CmdSetViewport({viewport});
}

} // namespace myvk

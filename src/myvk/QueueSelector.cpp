#include "QueueSelector.hpp"
#include "Surface.hpp"

namespace myvk {

GraphicsPresentQueueSelector::GraphicsPresentQueueSelector(std::shared_ptr<Queue> *graphics_queue,
                                                           const std::shared_ptr<Surface> &surface,
                                                           std::shared_ptr<PresentQueue> *present_queue)
    : m_graphics_queue(graphics_queue), m_surface(surface), m_present_queue(present_queue) {}

bool GraphicsPresentQueueSelector::operator()(
    const std::shared_ptr<PhysicalDevice> &physical_device, std::vector<QueueSelection> *const out_queue_selections,
    std::vector<PresentQueueSelection> *const out_present_queue_selections) const {

	const auto &families = physical_device->GetQueueFamilyProperties();
	if (families.empty())
		return false;

	myvk::PresentQueueSelection present_queue = {m_present_queue, m_surface, UINT32_MAX};
	myvk::QueueSelection graphics_queue = {m_graphics_queue, UINT32_MAX};

	// main queue and present queue
	for (uint32_t i = 0; i < families.size(); ++i) {
		VkQueueFlags flags = families[i].queueFlags;
		if ((flags & VK_QUEUE_GRAPHICS_BIT) && (flags & VK_QUEUE_TRANSFER_BIT)) {
			graphics_queue.family = i;
			graphics_queue.index_specifier = 0;

			if (physical_device->GetSurfaceSupport(i, present_queue.surface)) {
				present_queue.family = i;
				present_queue.index_specifier = 0;
				break;
			}
		}
	}

	// present queue fallback
	if (present_queue.family == UINT32_MAX)
		for (uint32_t i = 0; i < families.size(); ++i) {
			if (physical_device->GetSurfaceSupport(i, present_queue.surface)) {
				present_queue.family = i;
				present_queue.index_specifier = 0;
				break;
			}
		}

	(*out_queue_selections) = {graphics_queue};
	(*out_present_queue_selections) = {present_queue};

	return (~graphics_queue.family) && (~present_queue.family);
}
} // namespace myvk

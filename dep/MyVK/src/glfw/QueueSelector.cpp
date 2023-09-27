#include "myvk/QueueSelector.hpp"
#include "myvk/PhysicalDevice.hpp"
#include "myvk/Surface.hpp"
#include <optional>

namespace myvk {

GenericPresentQueueSelector::GenericPresentQueueSelector(Ptr<Queue> *p_generic_queue, const Ptr<Surface> &surface,
                                                         Ptr<PresentQueue> *p_present_queue)
    : m_p_generic_queue(p_generic_queue), m_surface(surface), m_p_present_queue(p_present_queue) {}

std::vector<QueueSelection>
GenericPresentQueueSelector::operator()(const Ptr<const PhysicalDevice> &physical_device) const {
	const auto &families = physical_device->GetQueueFamilyProperties();
	if (families.empty())
		return {};

	std::optional<uint32_t> generic_queue_family, present_queue_family;

	// generic queue and present queue
	for (uint32_t i = 0; i < families.size(); ++i) {
		VkQueueFlags flags = families[i].queueFlags;
		if ((flags & VK_QUEUE_GRAPHICS_BIT) && (flags & VK_QUEUE_TRANSFER_BIT) && (flags & VK_QUEUE_COMPUTE_BIT)) {
			generic_queue_family = i;

			if (physical_device->GetQueueSurfaceSupport(i, m_surface)) {
				present_queue_family = i;
				break;
			}
		}
	}
	// present queue fallback
	if (!present_queue_family.has_value())
		for (uint32_t i = 0; i < families.size(); ++i) {
			if (physical_device->GetQueueSurfaceSupport(i, m_surface)) {
				present_queue_family = i;
				break;
			}
		}

	if (generic_queue_family.has_value() && present_queue_family.has_value()) {
		return {{m_surface, m_p_present_queue, present_queue_family.value(), 0},
		        {m_p_generic_queue, generic_queue_family.value(), 0}};
	}
	return {};
}

GenericPresentTransferQueueSelector::GenericPresentTransferQueueSelector(Ptr<Queue> *p_generic_queue,
                                                                         Ptr<Queue> *p_transfer_queue,
                                                                         const Ptr<Surface> &surface,
                                                                         Ptr<PresentQueue> *p_present_queue)
    : m_p_generic_queue(p_generic_queue), m_p_transfer_queue(p_transfer_queue), m_surface(surface),
      m_p_present_queue(p_present_queue) {}

std::vector<QueueSelection>
GenericPresentTransferQueueSelector::operator()(const Ptr<const PhysicalDevice> &physical_device) const {

	const auto &families = physical_device->GetQueueFamilyProperties();
	if (families.empty())
		return {};

	std::optional<uint32_t> generic_queue_family, transfer_queue_family, present_queue_family;

	// generic queue and present queue
	for (uint32_t i = 0; i < families.size(); ++i) {
		VkQueueFlags flags = families[i].queueFlags;
		if ((flags & VK_QUEUE_GRAPHICS_BIT) && (flags & VK_QUEUE_TRANSFER_BIT) && (flags & VK_QUEUE_COMPUTE_BIT)) {
			generic_queue_family = i;
			transfer_queue_family = i;

			if (physical_device->GetQueueSurfaceSupport(i, m_surface)) {
				present_queue_family = i;
				break;
			}
		}
	}

	// find standalone transfer queue
	for (uint32_t i = 0; i < families.size(); ++i) {
		VkQueueFlags flags = families[i].queueFlags;
		if ((flags & VK_QUEUE_TRANSFER_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT) && !(flags & VK_QUEUE_COMPUTE_BIT)) {
			transfer_queue_family = i;
		}
	}

	// present queue fallback
	if (!present_queue_family.has_value())
		for (uint32_t i = 0; i < families.size(); ++i) {
			if (physical_device->GetQueueSurfaceSupport(i, m_surface)) {
				present_queue_family = i;
				break;
			}
		}

	if (generic_queue_family.has_value() && transfer_queue_family.has_value() && present_queue_family.has_value())
		return {{m_surface, m_p_present_queue, present_queue_family.value(), 0},
		        {m_p_generic_queue, generic_queue_family.value(), 0},
		        {m_p_transfer_queue, transfer_queue_family.value(), 1}};
	return {};
}

} // namespace myvk

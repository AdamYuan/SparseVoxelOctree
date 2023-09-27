#include "myvk/QueueSelector.hpp"
#include "myvk/Queue.hpp"
#include <optional>

namespace myvk {

std::vector<QueueSelection> GenericQueueSelector::operator()(const Ptr<const PhysicalDevice> &physical_device) const {
	const auto &families = physical_device->GetQueueFamilyProperties();
	if (families.empty())
		return {};

	for (uint32_t i = 0; i < families.size(); ++i) {
		VkQueueFlags flags = families[i].queueFlags;
		if ((flags & VK_QUEUE_GRAPHICS_BIT) && (flags & VK_QUEUE_TRANSFER_BIT) && (flags & VK_QUEUE_COMPUTE_BIT))
			return {{m_p_generic_queue, i, 0}};
	}
	return {};
}

} // namespace myvk

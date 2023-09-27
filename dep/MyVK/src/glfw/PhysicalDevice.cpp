#include "myvk/PhysicalDevice.hpp"
#include "myvk/Surface.hpp"

namespace myvk {
bool PhysicalDevice::GetQueueSurfaceSupport(uint32_t queue_family_index, const Ptr<Surface> &surface) const {
	VkBool32 support;
	if (vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, queue_family_index, surface->GetHandle(), &support) !=
	    VK_SUCCESS)
		return false;
	return support;
}
} // namespace myvk

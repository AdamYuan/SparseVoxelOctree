#ifndef MYVK_QUEUE_QUERY_HPP
#define MYVK_QUEUE_QUERY_HPP

#include <memory>
#include <vector>
#include <vk_queue_selector.h>
#include <volk.h>

namespace myvk {
class Queue;
class PresentQueue;
class Device;
class Surface;
class PhysicalDevice;

class QueueRequirement {
private:
	VkQueueFlags m_required_flags;
	float m_priority;
	std::shared_ptr<Queue> *m_out_queue;
	std::shared_ptr<Surface> m_surface_ptr;
	std::shared_ptr<PresentQueue> *m_out_present_queue;

	friend class DeviceCreateInfo;

public:
	QueueRequirement(VkQueueFlags flags, std::shared_ptr<Queue> *out_queue, float priority = 1.0f);

	QueueRequirement(VkQueueFlags flags, std::shared_ptr<Queue> *out_queue,
	                 std::shared_ptr<Surface> present_queue_surface, std::shared_ptr<PresentQueue> *out_present_queue,
	                 float priority = 1.0f);
};

class DeviceCreateInfo {
private:
	std::shared_ptr<PhysicalDevice> m_physical_device_ptr;

	std::vector<std::shared_ptr<Queue> *> m_out_queues;
	std::vector<std::shared_ptr<Surface>> m_surface_ptrs;
	std::vector<std::shared_ptr<PresentQueue> *> m_out_present_queues;
	VqsQuery m_query{nullptr};

	std::vector<const char *> m_extensions;
	bool m_extensions_support{true};

	bool m_use_allocator{true}, m_use_pipeline_cache{true};

	void enumerate_device_queue_create_infos(std::vector<VkDeviceQueueCreateInfo> *out_create_infos,
	                                         std::vector<float> *out_priorities) const;

	void fetch_queues(const std::shared_ptr<Device> &device) const;

	friend class Device;

public:
	void Initialize(const std::shared_ptr<PhysicalDevice> &physical_device,
	                const std::vector<QueueRequirement> &queue_requirements,
	                const std::vector<const char *> &extensions, bool use_allocator = true,
	                bool use_pipeline_cache = true);

	bool QueueSupport() const { return m_query; }
	bool ExtensionSupport() const { return m_extensions_support; }

	~DeviceCreateInfo();
};
} // namespace myvk

#endif

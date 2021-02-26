#include "DeviceCreateInfo.hpp"

#include "Queue.hpp"
#include "Surface.hpp"
#include <map>
#include <memory>
#include <set>
#include <string>

namespace myvk {
void DeviceCreateInfo::Initialize(const std::shared_ptr<PhysicalDevice> &physical_device,
                                  const QueueSelectorFunc &queue_selector_func,
                                  const std::vector<const char *> &extensions, bool use_allocator,
                                  bool use_pipeline_cache) {
	m_physical_device_ptr = physical_device;
	m_use_allocator = use_allocator;
	m_use_pipeline_cache = use_pipeline_cache;

	// CHECK EXTENSIONS
	m_extensions = extensions;
	{
		uint32_t extension_count;
		vkEnumerateDeviceExtensionProperties(m_physical_device_ptr->GetHandle(), nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> extension_properties(extension_count);
		vkEnumerateDeviceExtensionProperties(m_physical_device_ptr->GetHandle(), nullptr, &extension_count,
		                                     extension_properties.data());

		std::set<std::string> extension_set(m_extensions.begin(), m_extensions.end());
		for (const VkExtensionProperties &i : extension_properties) {
			extension_set.erase(i.extensionName);
		}
		m_extension_support = extension_set.empty();
	}

	// PROCESS QUEUES
	m_queue_selections.clear();
	m_present_queue_selections.clear();
	m_queue_support = queue_selector_func(physical_device, &m_queue_selections, &m_present_queue_selections);
	for (auto &i : m_queue_selections)
		i.queue_index %= physical_device->GetQueueFamilyProperties()[i.family_index].queueCount;
	for (auto &i : m_present_queue_selections)
		i.queue_index %= physical_device->GetQueueFamilyProperties()[i.family_index].queueCount;
}

void DeviceCreateInfo::enumerate_device_queue_create_infos(std::vector<VkDeviceQueueCreateInfo> *out_create_infos,
                                                           std::vector<float> *out_priorities) const {
	out_create_infos->clear();
	out_priorities->clear();

	std::map<uint32_t, std::set<uint32_t>> queue_map;
	for (const auto &i : m_present_queue_selections)
		queue_map[i.family_index].insert(i.queue_index);
	for (const auto &i : m_queue_selections)
		queue_map[i.family_index].insert(i.queue_index);

	if (queue_map.empty())
		return;

	uint32_t max_queue_count = 0;
	for (const auto &i : queue_map) {
		if (i.second.size() > max_queue_count)
			max_queue_count = i.second.size();
	}
	out_priorities->resize(max_queue_count, 1.0f);

	for (const auto &i : queue_map) {
		VkDeviceQueueCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		info.queueFamilyIndex = i.first;
		info.queueCount = i.second.size();
		info.pQueuePriorities = out_priorities->data();
		out_create_infos->push_back(info);
	}
}

void DeviceCreateInfo::fetch_queues(const std::shared_ptr<Device> &device) const {
	std::map<std::pair<uint32_t, uint32_t>, std::pair<std::shared_ptr<Queue>, std::shared_ptr<PresentQueue>>> queue_map;

	// process all present queue first
	for (const auto &i : m_present_queue_selections) {
		std::pair<uint32_t, uint32_t> queue_id = {i.family_index, i.queue_index};
		if (queue_map.find(queue_id) != queue_map.end())
			continue;
		std::shared_ptr<PresentQueue> present_queue =
		    PresentQueue::create(device, i.surface, i.family_index, i.queue_index);
		queue_map[queue_id].first = present_queue;
		queue_map[queue_id].second = present_queue;
	}

	// fallback as regular queue
	for (const auto &i : m_queue_selections) {
		std::pair<uint32_t, uint32_t> queue_id = {i.family_index, i.queue_index};
		if (queue_map.find(queue_id) != queue_map.end())
			continue;
		queue_map[queue_id].first = Queue::create(device, i.family_index, i.queue_index);
	}

	// set queue target
	for (const auto &i : m_queue_selections) {
		(*i.target) = queue_map[{i.family_index, i.queue_index}].first;
	}
	for (const auto &i : m_present_queue_selections) {
		(*i.target) = queue_map[{i.family_index, i.queue_index}].second;
	}
}

} // namespace myvk

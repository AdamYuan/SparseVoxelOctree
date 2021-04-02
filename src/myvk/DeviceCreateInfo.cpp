#include "DeviceCreateInfo.hpp"

#include "Queue.hpp"
#include "Surface.hpp"
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
	generate_queue_creations();
}

void DeviceCreateInfo::generate_queue_creations() {
	m_queue_creations.clear();
	// transform index_specifiers to queue_indices
	std::map<uint32_t, std::set<uint32_t>> queue_sets;
	std::map<uint32_t, std::map<uint32_t, uint32_t>> queue_tables;

	for (const auto &i : m_present_queue_selections)
		queue_sets[i.family].insert(i.index_specifier);
	for (const auto &i : m_queue_selections)
		queue_sets[i.family].insert(i.index_specifier);

	for (const auto &i : queue_sets) {
		uint32_t queue_count = m_physical_device_ptr->GetQueueFamilyProperties()[i.first].queueCount;
		m_queue_creations[i.first].resize(std::min((uint32_t)i.second.size(), queue_count));

		std::map<uint32_t, uint32_t> table;
		uint32_t cnt = 0;
		for (uint32_t x : i.second)
			table[x] = (cnt++) % queue_count;
		queue_tables[i.first] = std::move(table);
	}

	for (const auto &i : m_queue_selections) {
		uint32_t queue_index = queue_tables[i.family][i.index_specifier];
		m_queue_creations[i.family][queue_index].first.push_back(&i);
	}

	for (const auto &i : m_present_queue_selections) {
		uint32_t queue_index = queue_tables[i.family][i.index_specifier];
		m_queue_creations[i.family][queue_index].second.push_back(&i);
	}
}

void DeviceCreateInfo::enumerate_device_queue_create_infos(std::vector<VkDeviceQueueCreateInfo> *out_create_infos,
                                                           std::vector<float> *out_priorities) const {
	out_create_infos->clear();
	out_priorities->clear();

	if (m_queue_creations.empty())
		return;

	uint32_t max_queue_count = 0;
	for (const auto &i : m_queue_creations) {
		if (i.second.size() > max_queue_count)
			max_queue_count = i.second.size();
	}
	out_priorities->resize(max_queue_count, 1.0f);

	for (const auto &i : m_queue_creations) {
		VkDeviceQueueCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		info.queueFamilyIndex = i.first;
		info.queueCount = i.second.size();
		info.pQueuePriorities = out_priorities->data();
		out_create_infos->push_back(info);
	}
}

void DeviceCreateInfo::fetch_queues(const std::shared_ptr<Device> &device) const {
	for (const auto &creation : m_queue_creations) {
		uint32_t family = creation.first;
		for (uint32_t index = 0; index < creation.second.size(); ++index) {
			std::shared_ptr<UniqueQueue> unique_queue = UniqueQueue::Create(device, family, index);
			for (const QueueSelection *selection : creation.second[index].first)
				(*selection->target) = Queue::Create(unique_queue);
			for (const PresentQueueSelection *selection : creation.second[index].second)
				(*selection->target) = PresentQueue::Create(unique_queue, selection->surface);
		}
	}
}

} // namespace myvk

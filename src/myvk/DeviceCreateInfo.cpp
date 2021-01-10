#include "DeviceCreateInfo.hpp"

#include "PhysicalDevice.hpp"
#include "Queue.hpp"
#include "Surface.hpp"
#include "vk_queue_selector.h"
#include <map>
#include <memory>
#include <set>
#include <string>

namespace myvk {
QueueRequirement::QueueRequirement(VkQueueFlags flags, std::shared_ptr<Queue> *out_queue, float priority)
    : m_required_flags{flags}, m_priority{priority}, m_surface_ptr{nullptr}, m_out_queue{out_queue},
      m_out_present_queue{nullptr} {}

QueueRequirement::QueueRequirement(VkQueueFlags flags, std::shared_ptr<Queue> *out_queue,
                                   std::shared_ptr<Surface> present_queue_surface,
                                   std::shared_ptr<PresentQueue> *out_present_queue, float priority)
    : m_required_flags{flags}, m_priority{priority}, m_surface_ptr{std::move(present_queue_surface)},
      m_out_queue{out_queue}, m_out_present_queue{out_present_queue} {}

void DeviceCreateInfo::Initialize(const std::shared_ptr<PhysicalDevice> &physical_device,
                                  const std::vector<QueueRequirement> &queue_requirements,
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
		m_extensions_support = extension_set.empty();
	}

	// PROCESS QUEUES
	if (m_query) {
		vqsDestroyQuery(m_query);
		m_query = nullptr;
	}
	m_out_queues.clear();
	m_out_present_queues.clear();
	m_surface_ptrs.clear();

	std::vector<VqsQueueRequirements> vqs_queue_requirements;
	for (const auto &i : queue_requirements) {
		vqs_queue_requirements.push_back(
		    {i.m_required_flags, i.m_priority, i.m_surface_ptr ? i.m_surface_ptr->GetHandle() : nullptr});
		m_out_queues.push_back(i.m_out_queue);
		m_out_present_queues.push_back(i.m_out_present_queue);
		m_surface_ptrs.push_back(i.m_surface_ptr);
	}

	VqsVulkanFunctions vk_funcs{};
	vk_funcs.vkGetPhysicalDeviceQueueFamilyProperties = vkGetPhysicalDeviceQueueFamilyProperties;
	vk_funcs.vkGetPhysicalDeviceSurfaceSupportKHR = vkGetPhysicalDeviceSurfaceSupportKHR;

	VqsQueryCreateInfo create_info{};
	create_info.physicalDevice = physical_device->GetHandle();
	create_info.pVulkanFunctions = &vk_funcs;
	create_info.queueRequirementCount = vqs_queue_requirements.size();
	create_info.pQueueRequirements = vqs_queue_requirements.data();

	if (vqsCreateQuery(&create_info, &m_query) != VK_SUCCESS) {
		m_query = nullptr;
	}
}

void DeviceCreateInfo::enumerate_device_queue_create_infos(std::vector<VkDeviceQueueCreateInfo> *out_create_infos,
                                                           std::vector<float> *out_priorities) const {
	out_create_infos->clear();
	out_priorities->clear();
	uint32_t create_info_count, priority_count;
	vqsEnumerateDeviceQueueCreateInfos(m_query, &create_info_count, nullptr, &priority_count, nullptr);
	out_create_infos->resize(create_info_count);
	out_priorities->resize(priority_count);
	vqsEnumerateDeviceQueueCreateInfos(m_query, &create_info_count, out_create_infos->data(), &priority_count,
	                                   out_priorities->data());
}

void DeviceCreateInfo::fetch_queues(const std::shared_ptr<Device> &device) const {
	std::vector<VqsQueueSelection> selections(m_out_queues.size());
	vqsGetQueueSelections(m_query, selections.data());

	std::map<std::pair<uint32_t, uint32_t>, std::pair<std::shared_ptr<Queue>, std::shared_ptr<PresentQueue>>> queues;

	// process all present queue first
	for (uint32_t i = 0; i < m_out_queues.size(); ++i) {
		std::pair<uint32_t, uint32_t> queue_id = {selections[i].presentQueueFamilyIndex,
		                                          selections[i].presentQueueIndex};
		if (queues.find(queue_id) != queues.end())
			continue;
		if (m_out_present_queues[i]) {
			std::shared_ptr<PresentQueue> present_queue = PresentQueue::create(
			    device, m_surface_ptrs[i], selections[i].presentQueueFamilyIndex, selections[i].presentQueueIndex);
			queues[queue_id].first = present_queue;
			queues[queue_id].second = present_queue;
		}
	}

	// fallback as regular queue
	for (uint32_t i = 0; i < m_out_queues.size(); ++i) {
		std::pair<uint32_t, uint32_t> queue_id = {selections[i].queueFamilyIndex, selections[i].queueIndex};
		if (queues.find(queue_id) != queues.end())
			continue;
		queues[queue_id].first = Queue::create(device, selections[i].queueFamilyIndex, selections[i].queueIndex);
	}

	for (uint32_t i = 0; i < m_out_queues.size(); ++i) {
		(*m_out_queues[i]) = queues[{selections[i].queueFamilyIndex, selections[i].queueIndex}].first;
		if (m_out_present_queues[i])
			(*m_out_present_queues[i]) =
			    queues[{selections[i].presentQueueFamilyIndex, selections[i].presentQueueIndex}].second;
	}
}

DeviceCreateInfo::~DeviceCreateInfo() {
	if (m_query)
		vqsDestroyQuery(m_query);
}

} // namespace myvk

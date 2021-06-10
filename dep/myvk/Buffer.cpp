#include "Buffer.hpp"

#include <memory>
#include <set>

namespace myvk {
std::shared_ptr<Buffer> Buffer::Create(const std::shared_ptr<Device> &device, VkDeviceSize size,
                                       VmaMemoryUsage memory_usage, VkBufferUsageFlags buffer_usage,
                                       const std::vector<std::shared_ptr<Queue>> &access_queues) {
	VkBufferCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.size = size;
	create_info.usage = buffer_usage;

	return Create(device, create_info, memory_usage, access_queues);
}

std::shared_ptr<Buffer> Buffer::Create(const std::shared_ptr<Device> &device, const VkBufferCreateInfo &create_info,
                                       VmaMemoryUsage memory_usage,
                                       const std::vector<std::shared_ptr<Queue>> &access_queues) {
	std::shared_ptr<Buffer> ret = std::make_shared<Buffer>();
	ret->m_device_ptr = device;
	ret->m_size = create_info.size;

	std::set<uint32_t> queue_family_set;
	for (auto &i : access_queues)
		queue_family_set.insert(i->GetFamilyIndex());
	std::vector<uint32_t> queue_families{queue_family_set.begin(), queue_family_set.end()};

	VkBufferCreateInfo new_info{create_info};
	if (queue_families.size() <= 1) {
		new_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	} else {
		new_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
		new_info.queueFamilyIndexCount = queue_families.size();
		new_info.pQueueFamilyIndices = queue_families.data();
	}

	VmaAllocationCreateInfo alloc_create_info = {};
	alloc_create_info.usage = memory_usage;

	if (vmaCreateBuffer(device->GetAllocatorHandle(), &new_info, &alloc_create_info, &ret->m_buffer, &ret->m_allocation,
	                    nullptr) != VK_SUCCESS)
		return nullptr;

	return ret;
}

std::shared_ptr<Buffer> Buffer::CreateStaging(const std::shared_ptr<Device> &device, VkDeviceSize size,
                                              const std::vector<std::shared_ptr<Queue>> &access_queues) {
	return Create(device, size, VMA_MEMORY_USAGE_CPU_ONLY, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, access_queues);
}

void *Buffer::Map() const {
	void *ret;
	vmaMapMemory(m_device_ptr->GetAllocatorHandle(), m_allocation, &ret);
	return ret;
}

void Buffer::Unmap() const { vmaUnmapMemory(m_device_ptr->GetAllocatorHandle(), m_allocation); }

Buffer::~Buffer() {
	if (m_buffer)
		vmaDestroyBuffer(m_device_ptr->GetAllocatorHandle(), m_buffer, m_allocation);
}
}; // namespace myvk

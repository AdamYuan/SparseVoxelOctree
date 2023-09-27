#ifndef MYVK_BUFFER_HPP
#define MYVK_BUFFER_HPP

#include "BufferBase.hpp"
#include "Queue.hpp"
#include "vk_mem_alloc.h"
#include "volk.h"

namespace myvk {
class Buffer : public BufferBase {
private:
	Ptr<Device> m_device_ptr;

	VmaAllocation m_allocation{VK_NULL_HANDLE};
	void *m_mapped_ptr{};

public:
	static Ptr<Buffer> Create(const Ptr<Device> &device, const VkBufferCreateInfo &create_info,
	                          VmaAllocationCreateFlags allocation_flags,
	                          VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO,
	                          const std::vector<Ptr<Queue>> &access_queues = {});

	static Ptr<Buffer> Create(const Ptr<Device> &device, VkDeviceSize size, VmaAllocationCreateFlags allocation_flags,
	                          VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO,
	                          const std::vector<Ptr<Queue>> &access_queues = {});

	template <typename Iter>
	static Ptr<Buffer> CreateStaging(const Ptr<Device> &device, Iter begin, Iter end,
	                                 const std::vector<Ptr<Queue>> &access_queues = {}) {
		using T = typename std::iterator_traits<Iter>::value_type;
		Ptr<Buffer> ret =
		    Create(device, (end - begin) * sizeof(T),
		           VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		           VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO, access_queues);
		ret->UpdateData(begin, end, 0);
		return ret;
	}

	template <typename T, typename Writer>
	static Ptr<Buffer> CreateStaging(const Ptr<Device> &device, VkDeviceSize count, Writer &&writer_func,
	                                 bool sequential_write = true, const std::vector<Ptr<Queue>> &access_queues = {}) {
		Ptr<Buffer> ret = Create(device, count * sizeof(T),
		                         VMA_ALLOCATION_CREATE_MAPPED_BIT |
		                             (sequential_write ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
		                                               : VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT),
		                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO, access_queues);
		writer_func((T *)ret->m_mapped_ptr);
		return ret;
	}

	template <typename T>
	static Ptr<Buffer> CreateStaging(const Ptr<Device> &device, const T &data,
	                                 const std::vector<Ptr<Queue>> &access_queues = {}) {
		Ptr<Buffer> ret =
		    Create(device, sizeof(T),
		           VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		           VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO, access_queues);
		ret->UpdateData(data, 0);
		return ret;
	}

	inline void *GetMappedData() const { return m_mapped_ptr; }
	void *Map() const;
	void Unmap() const;

	template <typename Iter> inline void UpdateData(Iter begin, Iter end, uint32_t byte_offset = 0) const {
		using T = typename std::iterator_traits<Iter>::value_type;
		if (m_mapped_ptr) {
			std::copy(begin, end, (T *)((uint8_t *)GetMappedData() + byte_offset));
		} else {
			std::copy(begin, end, (T *)((uint8_t *)Map() + byte_offset));
			Unmap();
		}
	}

	template <typename T> inline void UpdateData(const T &data, uint32_t byte_offset = 0) const {
		if (m_mapped_ptr) {
			std::copy(&data, &data + 1, (T *)((uint8_t *)GetMappedData() + byte_offset));
		} else {
			std::copy(&data, &data + 1, (T *)((uint8_t *)Map() + byte_offset));
			Unmap();
		}
	}

	inline const Ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	~Buffer() override;
};
} // namespace myvk

#endif

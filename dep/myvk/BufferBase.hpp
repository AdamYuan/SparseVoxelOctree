#ifndef MYVK_BUFFER_BASE_HPP
#define MYVK_BUFFER_BASE_HPP

#include "DeviceObjectBase.hpp"
#include <volk.h>

namespace myvk {
struct BufferSubresourceRange {
	VkDeviceSize offset, size;
};

class BufferBase : public DeviceObjectBase {
protected:
	VkBuffer m_buffer{VK_NULL_HANDLE};
	VkDeviceSize m_size{0};

public:
	VkBuffer GetHandle() const { return m_buffer; }

	VkDeviceSize GetSize() const { return m_size; }

	std::vector<VkBufferMemoryBarrier> GetMemoryBarriers(const std::vector<BufferSubresourceRange> &regions,
	                                                     VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
	                                                     uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                                     uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;

	std::vector<VkBufferMemoryBarrier> GetSrcMemoryBarriers(const std::vector<VkBufferCopy> &regions,
	                                                        VkAccessFlags src_access_mask,
	                                                        VkAccessFlags dst_access_mask,
	                                                        uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                                        uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;

	std::vector<VkBufferMemoryBarrier> GetDstMemoryBarriers(const std::vector<VkBufferCopy> &regions,
	                                                        VkAccessFlags src_access_mask,
	                                                        VkAccessFlags dst_access_mask,
	                                                        uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                                        uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;

	std::vector<VkBufferMemoryBarrier> GetSrcMemoryBarriers(const std::vector<VkBufferImageCopy> &regions,
	                                                        VkAccessFlags src_access_mask,
	                                                        VkAccessFlags dst_access_mask,
	                                                        uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                                        uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;

	VkBufferMemoryBarrier GetMemoryBarrier(VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
	                                       uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                       uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;

	VkBufferMemoryBarrier GetMemoryBarrier(const BufferSubresourceRange &region, VkAccessFlags src_access_mask,
	                                       VkAccessFlags dst_access_mask,
	                                       uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                       uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;

	VkBufferMemoryBarrier GetSrcMemoryBarrier(const VkBufferCopy &region, VkAccessFlags src_access_mask,
	                                          VkAccessFlags dst_access_mask,
	                                          uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                          uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;

	VkBufferMemoryBarrier GetDstMemoryBarrier(const VkBufferCopy &region, VkAccessFlags src_access_mask,
	                                          VkAccessFlags dst_access_mask,
	                                          uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED,
	                                          uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED) const;
};
} // namespace myvk

#endif

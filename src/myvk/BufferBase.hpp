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
	VkBuffer m_buffer{nullptr};
	VkDeviceSize m_size{0};

public:
	VkBuffer GetHandle() const { return m_buffer; }

	VkDeviceSize GetSize() const { return m_size; }

	std::vector<VkBufferMemoryBarrier> GetMemoryBarriers(const std::vector<BufferSubresourceRange> &regions,
	                                                     VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
	                                                     const std::shared_ptr<myvk::Queue> &src_queue = nullptr,
	                                                     const std::shared_ptr<myvk::Queue> &dst_queue = nullptr) const;

	std::vector<VkBufferMemoryBarrier>
	GetSrcMemoryBarriers(const std::vector<VkBufferCopy> &regions, VkAccessFlags src_access_mask,
	                     VkAccessFlags dst_access_mask, const std::shared_ptr<myvk::Queue> &src_queue = nullptr,
	                     const std::shared_ptr<myvk::Queue> &dst_queue = nullptr) const;

	std::vector<VkBufferMemoryBarrier>
	GetDstMemoryBarriers(const std::vector<VkBufferCopy> &regions, VkAccessFlags src_access_mask,
	                     VkAccessFlags dst_access_mask, const std::shared_ptr<myvk::Queue> &src_queue = nullptr,
	                     const std::shared_ptr<myvk::Queue> &dst_queue = nullptr) const;

	std::vector<VkBufferMemoryBarrier>
	GetSrcMemoryBarriers(const std::vector<VkBufferImageCopy> &regions, VkAccessFlags src_access_mask,
	                     VkAccessFlags dst_access_mask, const std::shared_ptr<myvk::Queue> &src_queue = nullptr,
	                     const std::shared_ptr<myvk::Queue> &dst_queue = nullptr) const;

	VkBufferMemoryBarrier GetMemoryBarrier(VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
	                                       const std::shared_ptr<myvk::Queue> &src_queue = nullptr,
	                                       const std::shared_ptr<myvk::Queue> &dst_queue = nullptr) const;

	VkBufferMemoryBarrier GetMemoryBarrier(const BufferSubresourceRange &region, VkAccessFlags src_access_mask,
	                                       VkAccessFlags dst_access_mask,
	                                       const std::shared_ptr<myvk::Queue> &src_queue = nullptr,
	                                       const std::shared_ptr<myvk::Queue> &dst_queue = nullptr) const;

	VkBufferMemoryBarrier GetSrcMemoryBarrier(const VkBufferCopy &region, VkAccessFlags src_access_mask,
	                                          VkAccessFlags dst_access_mask,
	                                          const std::shared_ptr<myvk::Queue> &src_queue = nullptr,
	                                          const std::shared_ptr<myvk::Queue> &dst_queue = nullptr) const;

	VkBufferMemoryBarrier GetDstMemoryBarrier(const VkBufferCopy &region, VkAccessFlags src_access_mask,
	                                          VkAccessFlags dst_access_mask,
	                                          const std::shared_ptr<myvk::Queue> &src_queue = nullptr,
	                                          const std::shared_ptr<myvk::Queue> &dst_queue = nullptr) const;
};
} // namespace myvk

#endif

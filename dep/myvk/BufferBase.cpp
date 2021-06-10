#include "BufferBase.hpp"
#include "Queue.hpp"

namespace myvk {
#define MAKE_QUEUE_FAMILY                                                                                              \
	uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED, dst_queue_family = VK_QUEUE_FAMILY_IGNORED;                   \
	if (src_queue && dst_queue && src_queue->GetFamilyIndex() != dst_queue->GetFamilyIndex()) {                        \
		src_queue_family = src_queue->GetFamilyIndex();                                                                \
		dst_queue_family = dst_queue->GetFamilyIndex();                                                                \
	}

std::vector<VkBufferMemoryBarrier> BufferBase::GetMemoryBarriers(const std::vector<BufferSubresourceRange> &regions,
                                                                 VkAccessFlags src_access_mask,
                                                                 VkAccessFlags dst_access_mask,
                                                                 const std::shared_ptr<myvk::Queue> &src_queue,
                                                                 const std::shared_ptr<myvk::Queue> &dst_queue) const {
	MAKE_QUEUE_FAMILY
	std::vector<VkBufferMemoryBarrier> barriers(regions.size());
	for (uint32_t i = 0; i < regions.size(); ++i) {
		VkBufferMemoryBarrier &cur = barriers[i];
		cur.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		cur.size = regions[i].size;

		cur.buffer = m_buffer;
		cur.offset = regions[i].offset;
		cur.srcAccessMask = src_access_mask;
		cur.dstAccessMask = dst_access_mask;
		cur.srcQueueFamilyIndex = src_queue_family;
		cur.dstQueueFamilyIndex = dst_queue_family;
	}
	return barriers;
}

std::vector<VkBufferMemoryBarrier>
BufferBase::GetSrcMemoryBarriers(const std::vector<VkBufferCopy> &regions, VkAccessFlags src_access_mask,
                                 VkAccessFlags dst_access_mask, const std::shared_ptr<myvk::Queue> &src_queue,
                                 const std::shared_ptr<myvk::Queue> &dst_queue) const {
	MAKE_QUEUE_FAMILY
	std::vector<VkBufferMemoryBarrier> barriers(regions.size());
	for (uint32_t i = 0; i < regions.size(); ++i) {
		VkBufferMemoryBarrier &cur = barriers[i];
		cur.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		cur.size = regions[i].size;

		cur.buffer = m_buffer;
		cur.offset = regions[i].srcOffset;
		cur.srcAccessMask = src_access_mask;
		cur.dstAccessMask = dst_access_mask;
		cur.srcQueueFamilyIndex = src_queue_family;
		cur.dstQueueFamilyIndex = dst_queue_family;
	}
	return barriers;
}

std::vector<VkBufferMemoryBarrier>
BufferBase::GetDstMemoryBarriers(const std::vector<VkBufferCopy> &regions, VkAccessFlags src_access_mask,
                                 VkAccessFlags dst_access_mask, const std::shared_ptr<myvk::Queue> &src_queue,
                                 const std::shared_ptr<myvk::Queue> &dst_queue) const {
	MAKE_QUEUE_FAMILY
	std::vector<VkBufferMemoryBarrier> barriers(regions.size());
	for (uint32_t i = 0; i < regions.size(); ++i) {
		VkBufferMemoryBarrier &cur = barriers[i];
		cur.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		cur.size = regions[i].size;

		cur.buffer = m_buffer;
		cur.offset = regions[i].dstOffset;
		cur.srcAccessMask = src_access_mask;
		cur.dstAccessMask = dst_access_mask;
		cur.srcQueueFamilyIndex = src_queue_family;
		cur.dstQueueFamilyIndex = dst_queue_family;
	}
	return barriers;
}

std::vector<VkBufferMemoryBarrier>
BufferBase::GetSrcMemoryBarriers(const std::vector<VkBufferImageCopy> &regions, VkAccessFlags src_access_mask,
                                 VkAccessFlags dst_access_mask, const std::shared_ptr<myvk::Queue> &src_queue,
                                 const std::shared_ptr<myvk::Queue> &dst_queue) const {
	MAKE_QUEUE_FAMILY
	std::vector<VkBufferMemoryBarrier> barriers(regions.size());
	for (uint32_t i = 0; i < regions.size(); i++) {
		VkBufferMemoryBarrier &cur = barriers[i];
		cur.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		cur.size = (i == regions.size() - 1) ? m_size - regions[i].bufferOffset
		                                     : regions[i + 1].bufferOffset - regions[i].bufferOffset;

		cur.buffer = m_buffer;
		cur.offset = regions[i].bufferOffset;
		cur.srcAccessMask = src_access_mask;
		cur.dstAccessMask = dst_access_mask;

		cur.srcQueueFamilyIndex = src_queue_family;
		cur.dstQueueFamilyIndex = dst_queue_family;
	}
	return barriers;
}

VkBufferMemoryBarrier BufferBase::GetMemoryBarrier(const BufferSubresourceRange &region, VkAccessFlags src_access_mask,
                                                   VkAccessFlags dst_access_mask,
                                                   const std::shared_ptr<myvk::Queue> &src_queue,
                                                   const std::shared_ptr<myvk::Queue> &dst_queue) const {
	MAKE_QUEUE_FAMILY
	VkBufferMemoryBarrier ret = {};
	ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	ret.size = region.size;

	ret.buffer = m_buffer;
	ret.offset = region.offset;
	ret.srcAccessMask = src_access_mask;
	ret.dstAccessMask = dst_access_mask;
	ret.srcQueueFamilyIndex = src_queue_family;
	ret.dstQueueFamilyIndex = dst_queue_family;
	return ret;
}

VkBufferMemoryBarrier BufferBase::GetSrcMemoryBarrier(const VkBufferCopy &region, VkAccessFlags src_access_mask,
                                                      VkAccessFlags dst_access_mask,
                                                      const std::shared_ptr<myvk::Queue> &src_queue,
                                                      const std::shared_ptr<myvk::Queue> &dst_queue) const {
	MAKE_QUEUE_FAMILY
	VkBufferMemoryBarrier ret = {};
	ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	ret.size = region.size;

	ret.buffer = m_buffer;
	ret.offset = region.srcOffset;
	ret.srcAccessMask = src_access_mask;
	ret.dstAccessMask = dst_access_mask;
	ret.srcQueueFamilyIndex = src_queue_family;
	ret.dstQueueFamilyIndex = dst_queue_family;
	return ret;
}

VkBufferMemoryBarrier BufferBase::GetDstMemoryBarrier(const VkBufferCopy &region, VkAccessFlags src_access_mask,
                                                      VkAccessFlags dst_access_mask,
                                                      const std::shared_ptr<myvk::Queue> &src_queue,
                                                      const std::shared_ptr<myvk::Queue> &dst_queue) const {
	MAKE_QUEUE_FAMILY
	VkBufferMemoryBarrier ret = {};
	ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	ret.size = region.size;

	ret.buffer = m_buffer;
	ret.offset = region.dstOffset;
	ret.srcAccessMask = src_access_mask;
	ret.dstAccessMask = dst_access_mask;
	ret.srcQueueFamilyIndex = src_queue_family;
	ret.dstQueueFamilyIndex = dst_queue_family;
	return ret;
}

VkBufferMemoryBarrier BufferBase::GetMemoryBarrier(VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
                                                   const std::shared_ptr<myvk::Queue> &src_queue,
                                                   const std::shared_ptr<myvk::Queue> &dst_queue) const {
	MAKE_QUEUE_FAMILY
	VkBufferMemoryBarrier ret = {};
	ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	ret.size = m_size;
	ret.buffer = m_buffer;
	ret.offset = 0;
	ret.srcAccessMask = src_access_mask;
	ret.dstAccessMask = dst_access_mask;
	ret.srcQueueFamilyIndex = src_queue_family;
	ret.dstQueueFamilyIndex = dst_queue_family;
	return ret;
}
} // namespace myvk

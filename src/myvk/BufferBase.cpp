#include "BufferBase.hpp"

namespace myvk {
	std::vector<VkBufferMemoryBarrier>
	BufferBase::GetMemoryBarriers(const std::vector<BufferSubresourceRange> &regions, VkAccessFlags src_access_mask,
								  VkAccessFlags dst_access_mask) {
		std::vector<VkBufferMemoryBarrier> barriers(regions.size());
		for (uint32_t i = 0; i < regions.size(); ++i) {
			VkBufferMemoryBarrier &cur = barriers[i];
			cur.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			cur.size = regions[i].size;

			cur.buffer = m_buffer;
			cur.offset = regions[i].offset;
			cur.srcAccessMask = src_access_mask;
			cur.dstAccessMask = dst_access_mask;
			cur.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			cur.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		}
		return barriers;
	}

	std::vector<VkBufferMemoryBarrier>
	BufferBase::GetSrcMemoryBarriers(const std::vector<VkBufferCopy> &regions, VkAccessFlags src_access_mask,
									 VkAccessFlags dst_access_mask) {
		std::vector<VkBufferMemoryBarrier> barriers(regions.size());
		for (uint32_t i = 0; i < regions.size(); ++i) {
			VkBufferMemoryBarrier &cur = barriers[i];
			cur.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			cur.size = regions[i].size;

			cur.buffer = m_buffer;
			cur.offset = regions[i].srcOffset;
			cur.srcAccessMask = src_access_mask;
			cur.dstAccessMask = dst_access_mask;
			cur.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			cur.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		}
		return barriers;
	}

	std::vector<VkBufferMemoryBarrier>
	BufferBase::GetDstMemoryBarriers(const std::vector<VkBufferCopy> &regions, VkAccessFlags src_access_mask,
									 VkAccessFlags dst_access_mask) {
		std::vector<VkBufferMemoryBarrier> barriers(regions.size());
		for (uint32_t i = 0; i < regions.size(); ++i) {
			VkBufferMemoryBarrier &cur = barriers[i];
			cur.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			cur.size = regions[i].size;

			cur.buffer = m_buffer;
			cur.offset = regions[i].dstOffset;
			cur.srcAccessMask = src_access_mask;
			cur.dstAccessMask = dst_access_mask;
			cur.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			cur.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		}
		return barriers;
	}

	std::vector<VkBufferMemoryBarrier>
	BufferBase::GetSrcMemoryBarriers(const std::vector<VkBufferImageCopy> &regions, VkAccessFlags src_access_mask,
									 VkAccessFlags dst_access_mask) {
		std::vector<VkBufferMemoryBarrier> barriers(regions.size());
		for (uint32_t i = 0; i < regions.size(); i++) {
			VkBufferMemoryBarrier &cur = barriers[i];
			cur.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			cur.size = (i == regions.size() - 1) ? m_size - regions[i].bufferOffset : regions[i + 1].bufferOffset -
																					  regions[i].bufferOffset;

			cur.buffer = m_buffer;
			cur.offset = regions[i].bufferOffset;
			cur.srcAccessMask = src_access_mask;
			cur.dstAccessMask = dst_access_mask;

			cur.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			cur.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		}
		return barriers;
	}
}

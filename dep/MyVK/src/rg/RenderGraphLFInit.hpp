#pragma once

#include <myvk/Queue.hpp>

#include "RenderGraphAllocator.hpp"
#include "RenderGraphScheduler.hpp"

namespace myvk_rg::_details_ {

class RenderGraphLFInit {
private:
	std::vector<VkBufferMemoryBarrier2> m_pre_buffer_barriers, m_post_buffer_barriers;
	std::vector<VkImageMemoryBarrier2> m_pre_image_barriers, m_post_image_barriers;

	void insert_lf_buffer_barriers(const RenderGraphAllocator::IntBufferAlloc &buffer_alloc);
	void insert_lf_image_barriers(const RenderGraphAllocator::IntImageAlloc &image_alloc);

	static void cmd_lf_buffer_init(const myvk::Ptr<myvk::CommandBuffer> &command_buffer,
	                               const RenderGraphAllocator::IntBufferAlloc &buffer_alloc);
	static void cmd_lf_image_init(const myvk::Ptr<myvk::CommandBuffer> &command_buffer,
	                              const RenderGraphAllocator::IntImageAlloc &image_alloc);

public:
	void InitLastFrameResources(const myvk::Ptr<myvk::Queue> &queue, const RenderGraphAllocator &allocated);
};

} // namespace myvk_rg::_details_
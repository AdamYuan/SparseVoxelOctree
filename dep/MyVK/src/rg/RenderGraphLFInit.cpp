#include "RenderGraphLFInit.hpp"

#include "VkHelper.hpp"

namespace myvk_rg::_details_ {

void RenderGraphLFInit::InitLastFrameResources(const myvk::Ptr<myvk::Queue> &queue,
                                               const RenderGraphAllocator &allocated) {
	m_pre_buffer_barriers.clear();
	m_pre_image_barriers.clear();
	m_post_buffer_barriers.clear();
	m_post_image_barriers.clear();

	for (const auto &image_alloc : allocated.GetIntImageAllocVector())
		insert_lf_image_barriers(image_alloc);
	for (const auto &buffer_alloc : allocated.GetIntBufferAllocVector())
		insert_lf_buffer_barriers(buffer_alloc);

	if (m_pre_image_barriers.empty() && m_pre_buffer_barriers.empty() && m_post_image_barriers.empty() &&
	    m_post_buffer_barriers.empty())
		return;

	myvk::Ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(myvk::CommandPool::Create(queue));
	command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	{ // Pre barriers
		VkDependencyInfo dep_info = {VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
		dep_info.imageMemoryBarrierCount = m_pre_image_barriers.size();
		dep_info.pImageMemoryBarriers = m_pre_image_barriers.data();
		dep_info.bufferMemoryBarrierCount = m_pre_buffer_barriers.size();
		dep_info.pBufferMemoryBarriers = m_pre_buffer_barriers.data();
		vkCmdPipelineBarrier2(command_buffer->GetHandle(), &dep_info);
	}

	for (const auto &image_alloc : allocated.GetIntImageAllocVector())
		cmd_lf_image_init(command_buffer, image_alloc);
	for (const auto &buffer_alloc : allocated.GetIntBufferAllocVector())
		cmd_lf_buffer_init(command_buffer, buffer_alloc);

	{ // Post barriers
		VkDependencyInfo dep_info = {VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
		dep_info.imageMemoryBarrierCount = m_post_image_barriers.size();
		dep_info.pImageMemoryBarriers = m_post_image_barriers.data();
		dep_info.bufferMemoryBarrierCount = m_post_buffer_barriers.size();
		dep_info.pBufferMemoryBarriers = m_post_buffer_barriers.data();
		vkCmdPipelineBarrier2(command_buffer->GetHandle(), &dep_info);
	}

	command_buffer->End();
	auto fence = myvk::Fence::Create(queue->GetDevicePtr());
	command_buffer->Submit(fence);
	fence->Wait();
}

void RenderGraphLFInit::insert_lf_buffer_barriers(const RenderGraphAllocator::IntBufferAlloc &buffer_alloc) {
	const auto &buffer_info = buffer_alloc.GetBufferInfo();
	if (!buffer_info.p_last_frame_info)
		return;

	if (static_cast<const LastFrameBuffer *>(buffer_info.p_last_frame_info->lf_resource)->GetInitTransferFunc() ==
	    nullptr)
		return;

	auto last_references = RenderGraphScheduler::GetLastReferences<ResourceType::kBuffer>(buffer_info.last_references);

	VkPipelineStageFlags2 stage_flags = 0;
	VkAccessFlagBits2 access_flags = 0;
	for (const auto &ref : last_references) {
		stage_flags |= ref.p_input->GetUsagePipelineStages();
		access_flags |= UsageGetAccessFlags(ref.p_input->GetUsage());
	}

	VkBufferMemoryBarrier2 barrier = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2};
	barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	const auto push_barrier = [&buffer_alloc, &barrier](auto &barriers) {
		const myvk::Ptr<myvk::BufferBase> &myvk_buffer_0 = buffer_alloc.myvk_buffers[0];
		const myvk::Ptr<myvk::BufferBase> &myvk_buffer_1 = buffer_alloc.myvk_buffers[1];
		barrier.offset = 0;
		barrier.size = myvk_buffer_0->GetSize();
		barrier.buffer = myvk_buffer_0->GetHandle();
		barriers.push_back(barrier);
		if (myvk_buffer_1 != myvk_buffer_0) {
			barrier.buffer = myvk_buffer_1->GetHandle();
			barriers.push_back(barrier);
		}
	};

	{ // Pre barrier
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
		barrier.srcAccessMask = 0;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		push_barrier(m_pre_buffer_barriers);
	}
	{ // Post barrier
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
		barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		barrier.dstStageMask = stage_flags;
		barrier.dstAccessMask = access_flags;
		push_barrier(m_post_buffer_barriers);
	}
}

void RenderGraphLFInit::insert_lf_image_barriers(const RenderGraphAllocator::IntImageAlloc &image_alloc) {
	const auto &image_info = image_alloc.GetImageInfo();
	if (!image_info.p_last_frame_info)
		return;

	auto last_references = RenderGraphScheduler::GetLastReferences<ResourceType::kImage>(image_info.last_references);

	VkPipelineStageFlags2 stage_flags = 0;
	VkAccessFlagBits2 access_flags = 0;
	VkImageLayout layout = UsageGetImageLayout(last_references.front().p_input->GetUsage());
	for (const auto &ref : last_references) {
		stage_flags |= ref.p_input->GetUsagePipelineStages();
		auto usage = ref.p_input->GetUsage();
		access_flags |= UsageGetAccessFlags(usage);
		assert(UsageGetImageLayout(usage) == layout);
	}

	VkImageMemoryBarrier2 barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
	barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	const auto push_barrier = [&image_alloc, &image_info, &barrier](auto &barriers) {
		const myvk::Ptr<myvk::ImageBase> &myvk_image_0 = image_alloc.myvk_images[0];
		const myvk::Ptr<myvk::ImageBase> &myvk_image_1 = image_alloc.myvk_images[1];
		barrier.subresourceRange =
		    myvk_image_0->GetSubresourceRange(VkImageAspectFlagsFromVkFormat(image_info.image->GetFormat()));
		barrier.image = myvk_image_0->GetHandle();
		barriers.push_back(barrier);
		if (myvk_image_1 != myvk_image_0) {
			barrier.image = myvk_image_1->GetHandle();
			barriers.push_back(barrier);
		}
	};

	if (static_cast<const LastFrameImage *>(image_info.p_last_frame_info->lf_resource)->GetInitTransferFunc()) {
		{ // Pre barrier
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
			barrier.srcAccessMask = 0;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
			barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			push_barrier(m_pre_image_barriers);
		}
		{ // Post barrier
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
			barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.dstStageMask = stage_flags;
			barrier.dstAccessMask = access_flags;
			barrier.newLayout = layout;
			push_barrier(m_post_image_barriers);
		}
	} else {
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
		barrier.srcAccessMask = 0;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.dstStageMask = stage_flags;
		barrier.dstAccessMask = access_flags;
		barrier.newLayout = layout;
		push_barrier(m_pre_image_barriers);
	}
}
void RenderGraphLFInit::cmd_lf_buffer_init(const myvk::Ptr<myvk::CommandBuffer> &command_buffer,
                                           const RenderGraphAllocator::IntBufferAlloc &buffer_alloc) {
	const auto &buffer_info = buffer_alloc.GetBufferInfo();
	if (!buffer_info.p_last_frame_info)
		return;

	const auto &init_func =
	    static_cast<const LastFrameBuffer *>(buffer_info.p_last_frame_info->lf_resource)->GetInitTransferFunc();
	if (init_func == nullptr)
		return;

	const myvk::Ptr<myvk::BufferBase> &myvk_buffer_0 = buffer_alloc.myvk_buffers[0];
	const myvk::Ptr<myvk::BufferBase> &myvk_buffer_1 = buffer_alloc.myvk_buffers[1];
	init_func(command_buffer, myvk_buffer_0);
	if (myvk_buffer_1 != myvk_buffer_0)
		init_func(command_buffer, myvk_buffer_1);
}
void RenderGraphLFInit::cmd_lf_image_init(const myvk::Ptr<myvk::CommandBuffer> &command_buffer,
                                          const RenderGraphAllocator::IntImageAlloc &image_alloc) {
	const auto &image_info = image_alloc.GetImageInfo();
	if (!image_info.p_last_frame_info)
		return;

	const auto &init_func =
	    static_cast<const LastFrameImage *>(image_info.p_last_frame_info->lf_resource)->GetInitTransferFunc();
	if (init_func == nullptr)
		return;

	const myvk::Ptr<myvk::ImageBase> &myvk_image_0 = image_alloc.myvk_images[0];
	const myvk::Ptr<myvk::ImageBase> &myvk_image_1 = image_alloc.myvk_images[1];
	init_func(command_buffer, myvk_image_0);
	if (myvk_image_1 != myvk_image_0)
		init_func(command_buffer, myvk_image_1);
}

} // namespace myvk_rg::_details_

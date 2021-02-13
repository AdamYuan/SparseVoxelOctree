#include "CommandBuffer.hpp"
#include <algorithm>

namespace myvk {
std::shared_ptr<CommandBuffer> CommandBuffer::Create(const std::shared_ptr<CommandPool> &command_pool,
                                                     VkCommandBufferLevel level) {
	std::shared_ptr<CommandBuffer> ret = std::make_shared<CommandBuffer>();
	ret->m_command_pool_ptr = command_pool;

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = command_pool->GetHandle();
	alloc_info.level = level;
	alloc_info.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(command_pool->GetDevicePtr()->GetHandle(), &alloc_info, &ret->m_command_buffer) !=
	    VK_SUCCESS)
		return nullptr;
	return ret;
}

std::vector<std::shared_ptr<CommandBuffer>>
CommandBuffer::CreateMultiple(const std::shared_ptr<CommandPool> &command_pool, uint32_t count,
                              VkCommandBufferLevel level) {
	std::vector<std::shared_ptr<CommandBuffer>> ret(count);
	for (uint32_t i = 0; i < count; ++i) {
		ret[i] = std::make_shared<CommandBuffer>();
		ret[i]->m_command_pool_ptr = command_pool;
	}

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = command_pool->GetHandle();
	alloc_info.level = level;
	alloc_info.commandBufferCount = count;

	std::vector<VkCommandBuffer> handles(count);

	if (vkAllocateCommandBuffers(command_pool->GetDevicePtr()->GetHandle(), &alloc_info, handles.data()) != VK_SUCCESS)
		return {};

	for (uint32_t i = 0; i < count; ++i)
		ret[i]->m_command_buffer = handles[i];

	return ret;
}

CommandBuffer::~CommandBuffer() {
	if (m_command_buffer) {
		vkFreeCommandBuffers(m_command_pool_ptr->GetQueuePtr()->GetDevicePtr()->GetHandle(),
		                     m_command_pool_ptr->GetHandle(), 1, &m_command_buffer);
	}
}

VkResult CommandBuffer::Submit(const SemaphoreStageGroup &wait_semaphores, const SemaphoreGroup &signal_semaphores,
                               const std::shared_ptr<Fence> &fence) const {
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	info.commandBufferCount = 1;
	info.pCommandBuffers = &m_command_buffer;

	if (wait_semaphores.GetCount()) {
		info.waitSemaphoreCount = wait_semaphores.GetCount();
		info.pWaitSemaphores = wait_semaphores.GetSemaphoresPtr();
		info.pWaitDstStageMask = wait_semaphores.GetWaitStagesPtr();
	}

	if (signal_semaphores.GetCount()) {
		info.signalSemaphoreCount = signal_semaphores.GetCount();
		info.pSignalSemaphores = signal_semaphores.GetSemaphoresPtr();
	}

	std::lock_guard<std::mutex> lock_guard{m_command_pool_ptr->GetQueuePtr()->GetMutex()};
	return vkQueueSubmit(m_command_pool_ptr->GetQueuePtr()->GetHandle(), 1, &info,
	                     fence ? fence->GetHandle() : VK_NULL_HANDLE);
}

VkResult CommandBuffer::Submit(const std::shared_ptr<Fence> &fence) const {
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	info.commandBufferCount = 1;
	info.pCommandBuffers = &m_command_buffer;

	std::lock_guard<std::mutex> lock_guard{m_command_pool_ptr->GetQueuePtr()->GetMutex()};
	return vkQueueSubmit(m_command_pool_ptr->GetQueuePtr()->GetHandle(), 1, &info,
	                     fence ? fence->GetHandle() : VK_NULL_HANDLE);
}

VkResult CommandBuffer::Begin(VkCommandBufferUsageFlags usage) const {
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = usage;
	return vkBeginCommandBuffer(m_command_buffer, &begin_info);
}

VkResult CommandBuffer::End() const { return vkEndCommandBuffer(m_command_buffer); }

VkResult CommandBuffer::Reset(VkCommandBufferResetFlags flags) const {
	return vkResetCommandBuffer(m_command_buffer, flags);
}

void CommandBuffer::CmdBeginRenderPass(const std::shared_ptr<RenderPass> &render_pass,
                                       const std::shared_ptr<Framebuffer> &framebuffer,
                                       const std::vector<VkClearValue> &clear_values, const VkOffset2D &offset,
                                       const VkExtent2D &extent, VkSubpassContents subpass_contents) const {
	VkRenderPassBeginInfo render_begin_info = {};
	render_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_begin_info.renderPass = render_pass->GetHandle();
	render_begin_info.framebuffer = framebuffer->GetHandle();
	render_begin_info.renderArea.offset = offset;
	render_begin_info.renderArea.extent = extent;
	render_begin_info.clearValueCount = clear_values.size();
	render_begin_info.pClearValues = clear_values.data();

	vkCmdBeginRenderPass(m_command_buffer, &render_begin_info, subpass_contents);
}

void CommandBuffer::CmdBeginRenderPass(const std::shared_ptr<RenderPass> &render_pass,
                                       const std::shared_ptr<Framebuffer> &framebuffer,
                                       const std::vector<VkClearValue> &clear_values,
                                       VkSubpassContents subpass_contents) const {
	VkRenderPassBeginInfo render_begin_info = {};
	render_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_begin_info.renderPass = render_pass->GetHandle();
	render_begin_info.framebuffer = framebuffer->GetHandle();
	render_begin_info.renderArea.offset = {0, 0};
	render_begin_info.renderArea.extent = framebuffer->GetExtent();
	render_begin_info.clearValueCount = clear_values.size();
	render_begin_info.pClearValues = clear_values.data();

	vkCmdBeginRenderPass(m_command_buffer, &render_begin_info, subpass_contents);
}

void CommandBuffer::CmdEndRenderPass() const { vkCmdEndRenderPass(m_command_buffer); }

void CommandBuffer::CmdBindPipeline(const std::shared_ptr<PipelineBase> &pipeline) const {
	vkCmdBindPipeline(m_command_buffer, pipeline->GetBindPoint(), pipeline->GetHandle());
}

void CommandBuffer::CmdBindDescriptorSets(const std::vector<std::shared_ptr<DescriptorSet>> &descriptor_sets,
                                          const std::shared_ptr<PipelineLayout> &pipeline_layout,
                                          VkPipelineBindPoint pipeline_bind_point,
                                          const std::vector<uint32_t> &offsets) const {
	std::vector<VkDescriptorSet> handles(descriptor_sets.size());
	for (uint32_t i = 0; i < handles.size(); ++i)
		handles[i] = descriptor_sets[i]->GetHandle();

	vkCmdBindDescriptorSets(m_command_buffer, pipeline_bind_point, pipeline_layout->GetHandle(), 0, handles.size(),
	                        handles.data(), offsets.size(), offsets.data());
}

void CommandBuffer::CmdBindDescriptorSets(const std::vector<std::shared_ptr<DescriptorSet>> &descriptor_sets,
                                          const std::shared_ptr<PipelineBase> &pipeline,
                                          const std::vector<uint32_t> &offsets) const {
	CmdBindDescriptorSets(descriptor_sets, pipeline->GetPipelineLayoutPtr(), pipeline->GetBindPoint(), offsets);
}

void CommandBuffer::CmdBindVertexBuffer(const std::shared_ptr<BufferBase> &buffer, VkDeviceSize offset,
                                        uint32_t start_slot) const {
	VkBuffer handle = buffer->GetHandle();
	vkCmdBindVertexBuffers(m_command_buffer, start_slot, 1, &handle, &offset);
}

void CommandBuffer::CmdCopy(const std::shared_ptr<BufferBase> &src, const std::shared_ptr<BufferBase> &dst,
                            const std::vector<VkBufferCopy> &regions) const {
	vkCmdCopyBuffer(m_command_buffer, src->GetHandle(), dst->GetHandle(), regions.size(), regions.data());
}

void CommandBuffer::CmdCopy(const std::shared_ptr<BufferBase> &src, const std::shared_ptr<ImageBase> &dst,
                            const std::vector<VkBufferImageCopy> &regions, VkImageLayout layout) const {
	vkCmdCopyBufferToImage(m_command_buffer, src->GetHandle(), dst->GetHandle(), layout, regions.size(),
	                       regions.data());
}

void CommandBuffer::CmdCopy(const std::shared_ptr<ImageBase> &src, const std::shared_ptr<BufferBase> &dst,
                            const std::vector<VkBufferImageCopy> &regions, VkImageLayout layout) const {
	vkCmdCopyImageToBuffer(m_command_buffer, src->GetHandle(), layout, dst->GetHandle(), regions.size(),
	                       regions.data());
}

void CommandBuffer::CmdDraw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex,
                            uint32_t first_instance) const {
	vkCmdDraw(m_command_buffer, vertex_count, instance_count, first_vertex, first_instance);
}

void CommandBuffer::CmdDrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index,
                                   uint32_t vertex_offset, uint32_t first_instance) const {
	vkCmdDrawIndexed(m_command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
}

void CommandBuffer::CmdNextSubpass(VkSubpassContents subpass_contents) const {
	vkCmdNextSubpass(m_command_buffer, subpass_contents);
}

void CommandBuffer::CmdPipelineBarrier(VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
                                       const std::vector<VkMemoryBarrier> &memory_barriers,
                                       const std::vector<VkBufferMemoryBarrier> &buffer_memory_barriers,
                                       const std::vector<VkImageMemoryBarrier> &image_memory_barriers) const {
	vkCmdPipelineBarrier(m_command_buffer, src_stage, dst_stage, 0, memory_barriers.size(), memory_barriers.data(),
	                     buffer_memory_barriers.size(), buffer_memory_barriers.data(), image_memory_barriers.size(),
	                     image_memory_barriers.data());
}

void CommandBuffer::CmdBindIndexBuffer(const std::shared_ptr<BufferBase> &buffer, VkDeviceSize offset,
                                       VkIndexType type) const {
	vkCmdBindIndexBuffer(m_command_buffer, buffer->GetHandle(), offset, type);
}

void CommandBuffer::CmdSetViewport(const std::vector<VkViewport> &viewports) const {
	vkCmdSetViewport(m_command_buffer, 0, viewports.size(), viewports.data());
}

void CommandBuffer::CmdSetScissor(const std::vector<VkRect2D> &scissors) const {
	vkCmdSetScissor(m_command_buffer, 0, scissors.size(), scissors.data());
}

void CommandBuffer::CmdPushConstants(const std::shared_ptr<PipelineLayout> &pipeline_layout,
                                     VkShaderStageFlags shader_stage, uint32_t offset, uint32_t size,
                                     const void *data) const {
	vkCmdPushConstants(m_command_buffer, pipeline_layout->GetHandle(), shader_stage, offset, size, data);
}

void CommandBuffer::CmdBlitImage(const std::shared_ptr<ImageBase> &src, const std::shared_ptr<ImageBase> &dst,
                                 const VkImageBlit &blit, VkFilter filter) const {
	vkCmdBlitImage(m_command_buffer, src->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->GetHandle(),
	               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, filter);
}

void CommandBuffer::CmdGenerateMipmap2D(const std::shared_ptr<ImageBase> &image, VkPipelineStageFlags src_stage,
                                        VkPipelineStageFlags dst_stage, VkAccessFlags src_access_mask,
                                        VkAccessFlags dst_access_mask, VkImageLayout old_layout,
                                        VkImageLayout new_layout) const {
	if (image->GetMipLevels() == 1) {
		VkImageSubresourceLayers subresource = {};
		subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource.mipLevel = 0;
		subresource.baseArrayLayer = 0;
		subresource.layerCount = image->GetArrayLayers();
		CmdPipelineBarrier(
		    src_stage, dst_stage, {}, {},
		    image->GetMemoryBarriers({subresource}, src_access_mask, dst_access_mask, old_layout, new_layout));
		return;
	}
	int32_t mip_width = image->GetExtent().width, mip_height = image->GetExtent().height;
	for (uint32_t i = 1; i < image->GetMipLevels(); ++i) {
		VkImageBlit blit{};
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {mip_width, mip_height, (int32_t)image->GetExtent().depth};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = image->GetArrayLayers();
		mip_width = std::max(mip_width / 2, 1);
		mip_height = std::max(mip_height / 2, 1);
		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {mip_width, mip_height, (int32_t)image->GetExtent().depth};
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = image->GetArrayLayers();

		CmdPipelineBarrier(i == 1 ? src_stage : VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {},
		                   image->GetMemoryBarriers(
		                       {blit.srcSubresource}, i == 1 ? src_access_mask : VK_ACCESS_TRANSFER_WRITE_BIT,
		                       VK_ACCESS_TRANSFER_READ_BIT, i == 1 ? old_layout : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL));
		CmdPipelineBarrier(i == 1 ? src_stage : VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {},
		                   image->GetMemoryBarriers({blit.dstSubresource}, i == 1 ? src_access_mask : 0,
		                                            VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
		                                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));

		CmdBlitImage(image, image, blit, VK_FILTER_LINEAR);

		CmdPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, dst_stage, {}, {},
		                   image->GetMemoryBarriers({blit.srcSubresource}, VK_ACCESS_TRANSFER_READ_BIT, dst_access_mask,
		                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, new_layout));
		if (i == image->GetMipLevels() - 1) {
			CmdPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, dst_stage, {}, {},
			                   image->GetMemoryBarriers({blit.dstSubresource}, VK_ACCESS_TRANSFER_WRITE_BIT,
			                                            dst_access_mask, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			                                            new_layout));
		}
	}
}

void CommandBuffer::CmdClearColorImage(const std::shared_ptr<ImageBase> &image, VkImageLayout layout,
                                       const VkClearColorValue &color,
                                       const std::vector<VkImageSubresourceRange> &regions) const {
	vkCmdClearColorImage(m_command_buffer, image->GetHandle(), layout, &color, regions.size(), regions.data());
}

void CommandBuffer::CmdClearColorImage(const std::shared_ptr<ImageBase> &image, VkImageLayout layout,
                                       const VkClearColorValue &color) const {
	CmdClearColorImage(image, layout, color, {image->GetSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT)});
}

void CommandBuffer::CmdDispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) const {
	vkCmdDispatch(m_command_buffer, group_x, group_y, group_z);
}

void CommandBuffer::CmdDispatchIndirect(const std::shared_ptr<BufferBase> &buffer, VkDeviceSize offset) const {
	vkCmdDispatchIndirect(m_command_buffer, buffer->GetHandle(), offset);
}

void CommandBuffer::CmdResetQueryPool(const std::shared_ptr<QueryPool> &query_pool, uint32_t first_query,
                                      uint32_t query_count) const {
	vkCmdResetQueryPool(m_command_buffer, query_pool->GetHandle(), first_query, query_count);
}

void CommandBuffer::CmdResetQueryPool(const std::shared_ptr<QueryPool> &query_pool) const {
	vkCmdResetQueryPool(m_command_buffer, query_pool->GetHandle(), 0, query_pool->GetCount());
}

void CommandBuffer::CmdWriteTimestamp(VkPipelineStageFlagBits pipeline_stage,
                                      const std::shared_ptr<QueryPool> &query_pool, uint32_t query) const {
	vkCmdWriteTimestamp(m_command_buffer, pipeline_stage, query_pool->GetHandle(), query);
}

/*CommandBufferGroup::CommandBufferGroup(const std::vector<std::shared_ptr<CommandBuffer>> &command_buffers) {
    Initialize(command_buffers);
}

CommandBufferGroup::CommandBufferGroup(
    const std::initializer_list<std::shared_ptr<CommandBuffer>> &command_buffers) {
    Initialize(command_buffers);
}

void CommandBufferGroup::Initialize(const std::vector<std::shared_ptr<CommandBuffer>> &command_buffers) {
    m_command_buffers.clear();
    for (const auto &i : command_buffers) {
        m_command_buffers[i->GetCommandPoolPtr()->GetQueuePtr()->GetHandle()].push_back(i->GetHandle());
    }
}

VkResult
CommandBufferGroup::Submit(const SemaphoreStageGroup &wait_semaphores, const SemaphoreGroup &signal_semaphores,
                           const std::shared_ptr<Fence> &fence) const {
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    if (wait_semaphores.GetCount()) {
        info.waitSemaphoreCount = wait_semaphores.GetCount();
        info.pWaitSemaphores = wait_semaphores.GetSemaphoresPtr();
        info.pWaitDstStageMask = wait_semaphores.GetWaitStagesPtr();
    }

    if (signal_semaphores.GetCount()) {
        info.signalSemaphoreCount = signal_semaphores.GetCount();
        info.pSignalSemaphores = signal_semaphores.GetSemaphoresPtr();
    }

    for (const auto &i : m_command_buffers) {
        info.commandBufferCount = i.second.size();
        info.pCommandBuffers = i.second.data();
        VkResult res = vkQueueSubmit(i.first, 1, &info, fence ? fence->GetHandle() : VK_NULL_HANDLE);
        if (res != VK_SUCCESS) return res;
    }
    return VK_SUCCESS;
}*/
} // namespace myvk

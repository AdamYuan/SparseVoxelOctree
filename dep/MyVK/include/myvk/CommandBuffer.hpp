#ifndef MYVK_COMMAND_BUFFER_HPP
#define MYVK_COMMAND_BUFFER_HPP

#include "BufferBase.hpp"
#include "CommandPool.hpp"
#include "DescriptorSet.hpp"
#include "DeviceObjectBase.hpp"
#include "Fence.hpp"
#include "Framebuffer.hpp"
#include "ImagelessFramebuffer.hpp"
#include "PipelineBase.hpp"
#include "QueryPool.hpp"
#include "RenderPass.hpp"
#include "Semaphore.hpp"

#include "volk.h"
#include <map>
#include <memory>
#include <vector>

namespace myvk {
class CommandBuffer : public DeviceObjectBase {
private:
	Ptr<CommandPool> m_command_pool_ptr;

	VkCommandBuffer m_command_buffer{VK_NULL_HANDLE};

public:
	static Ptr<CommandBuffer> Create(const Ptr<CommandPool> &command_pool,
	                                 VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	static std::vector<Ptr<CommandBuffer>> CreateMultiple(const Ptr<CommandPool> &command_pool, uint32_t count,
	                                                      VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkResult Submit(const SemaphoreStageGroup &wait_semaphores = SemaphoreStageGroup(),
	                const SemaphoreGroup &signal_semaphores = SemaphoreGroup(),
	                const Ptr<Fence> &fence = nullptr) const;
	VkResult Submit(const Ptr<Fence> &fence = nullptr) const;

	VkResult Reset(VkCommandBufferResetFlags flags = 0) const;

	VkResult Begin(VkCommandBufferUsageFlags usage = 0) const;
	VkResult BeginSecondary(VkCommandBufferUsageFlags usage = 0) const;

	VkResult End() const;

	void CmdExecuteCommands(const std::vector<Ptr<CommandBuffer>> &command_buffers) const;
	void CmdExecuteCommand(const Ptr<CommandBuffer> &command_buffer) const;

	void CmdBeginRenderPass(const Ptr<RenderPass> &render_pass, const Ptr<Framebuffer> &framebuffer,
	                        const std::vector<VkClearValue> &clear_values, const VkOffset2D &offset,
	                        const VkExtent2D &extent,
	                        VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE) const;

	void CmdBeginRenderPass(const Ptr<RenderPass> &render_pass, const Ptr<Framebuffer> &framebuffer,
	                        const std::vector<VkClearValue> &clear_values,
	                        VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE) const;

	void CmdBeginRenderPass(const Ptr<RenderPass> &render_pass, const Ptr<ImagelessFramebuffer> &framebuffer,
	                        const std::vector<Ptr<ImageView>> &attachments,
	                        const std::vector<VkClearValue> &clear_values,
	                        VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE) const;

	void CmdEndRenderPass() const;

	void CmdBindPipeline(const Ptr<PipelineBase> &pipeline) const;

	void CmdBindDescriptorSets(const std::vector<Ptr<DescriptorSet>> &descriptor_sets, uint32_t first_set,
	                           const Ptr<PipelineLayout> &pipeline_layout, VkPipelineBindPoint pipeline_bind_point,
	                           const std::vector<uint32_t> &offsets = {}) const;
	void CmdBindDescriptorSets(const std::vector<Ptr<DescriptorSet>> &descriptor_sets,
	                           const Ptr<PipelineLayout> &pipeline_layout, VkPipelineBindPoint pipeline_bind_point,
	                           const std::vector<uint32_t> &offsets = {}) const;
	void CmdBindDescriptorSets(const std::vector<Ptr<DescriptorSet>> &descriptor_sets,
	                           const Ptr<PipelineBase> &pipeline, const std::vector<uint32_t> &offsets = {}) const;
	void CmdBindDescriptorSets(const std::vector<Ptr<DescriptorSet>> &descriptor_sets, uint32_t first_set,
	                           const Ptr<PipelineBase> &pipeline, const std::vector<uint32_t> &offsets = {}) const;

	void CmdBindVertexBuffer(const Ptr<BufferBase> &buffer, VkDeviceSize offset, uint32_t start_slot = 0) const;

	void CmdBindIndexBuffer(const Ptr<BufferBase> &buffer, VkDeviceSize offset, VkIndexType type) const;

	void CmdSetViewport(const std::vector<VkViewport> &viewports) const;

	void CmdSetScissor(const std::vector<VkRect2D> &scissors) const;

	void CmdPushConstants(const Ptr<PipelineLayout> &pipeline_layout, VkShaderStageFlags shader_stage, uint32_t offset,
	                      uint32_t size, const void *data) const;

	void CmdCopy(const Ptr<BufferBase> &src, const Ptr<BufferBase> &dst,
	             const std::vector<VkBufferCopy> &regions) const;

	void CmdCopy(const Ptr<BufferBase> &src, const Ptr<ImageBase> &dst, const std::vector<VkBufferImageCopy> &regions,
	             VkImageLayout dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) const;

	void CmdCopy(const Ptr<ImageBase> &src, const Ptr<BufferBase> &dst, const std::vector<VkBufferImageCopy> &regions,
	             VkImageLayout src_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) const;

	void CmdCopy(const Ptr<ImageBase> &src, const Ptr<ImageBase> &dst, const std::vector<VkImageCopy> &regions,
	             VkImageLayout src_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	             VkImageLayout dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) const;

	/*void CmdCopy(const Ptr<ImageView> &src_view, const Ptr<ImageView> &dst_view,
	             VkImageLayout src_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	             VkImageLayout dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) const; */

	void CmdDraw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) const;

	void CmdDrawIndirect(const Ptr<BufferBase> &buffer, VkDeviceSize offset, uint32_t draw_count,
	                     uint32_t stride = sizeof(VkDrawIndirectCommand)) const;

	void CmdDrawIndirectCount(const Ptr<BufferBase> &buffer, VkDeviceSize offset, const Ptr<BufferBase> &count_buffer,
	                          VkDeviceSize count_buffer_offset, uint32_t max_draw_count,
	                          uint32_t stride = sizeof(VkDrawIndirectCommand)) const;

	void CmdDrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset,
	                    uint32_t first_instance) const;

	void CmdDrawIndexedIndirect(const Ptr<BufferBase> &buffer, VkDeviceSize offset, uint32_t draw_count,
	                            uint32_t stride = sizeof(VkDrawIndexedIndirectCommand)) const;

	void CmdDrawIndexedIndirectCount(const Ptr<BufferBase> &buffer, VkDeviceSize offset,
	                                 const Ptr<BufferBase> &count_buffer, VkDeviceSize count_buffer_offset,
	                                 uint32_t max_draw_count,
	                                 uint32_t stride = sizeof(VkDrawIndexedIndirectCommand)) const;

	void CmdNextSubpass(VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE) const;

	void CmdPipelineBarrier(VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
	                        const std::vector<VkMemoryBarrier> &memory_barriers,
	                        const std::vector<VkBufferMemoryBarrier> &buffer_memory_barriers,
	                        const std::vector<VkImageMemoryBarrier> &image_memory_barriers) const;

	void CmdBlitImage(const Ptr<ImageBase> &src, const Ptr<ImageBase> &dst, const VkImageBlit &blit,
	                  VkFilter filter) const;

	void CmdBlitImage(const Ptr<myvk::ImageView> &src_view, const Ptr<myvk::ImageView> &dst_view,
	                  VkFilter filter) const;

	void CmdClearColorImage(const Ptr<ImageBase> &image, VkImageLayout layout, const VkClearColorValue &color,
	                        const std::vector<VkImageSubresourceRange> &regions) const;

	void CmdClearColorImage(const Ptr<ImageBase> &image, VkImageLayout layout,
	                        const VkClearColorValue &color = {}) const;

	void CmdGenerateMipmap2D(const Ptr<ImageBase> &image, VkPipelineStageFlags src_stage,
	                         VkPipelineStageFlags dst_stage, VkAccessFlags src_access_mask,
	                         VkAccessFlags dst_access_mask, VkImageLayout old_layout, VkImageLayout new_layout) const;

	void CmdDispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) const;

	void CmdDispatchIndirect(const Ptr<BufferBase> &buffer, VkDeviceSize offset = 0) const;

	void CmdResetQueryPool(const Ptr<QueryPool> &query_pool, uint32_t first_query, uint32_t query_count) const;

	void CmdResetQueryPool(const Ptr<QueryPool> &query_pool) const;

	void CmdWriteTimestamp(VkPipelineStageFlagBits pipeline_stage, const Ptr<QueryPool> &query_pool,
	                       uint32_t query) const;

	VkCommandBuffer GetHandle() const { return m_command_buffer; }

	const Ptr<CommandPool> &GetCommandPoolPtr() const { return m_command_pool_ptr; }

	const Ptr<Device> &GetDevicePtr() const override { return m_command_pool_ptr->GetDevicePtr(); };

	~CommandBuffer() override;
};

/*class CommandBufferGroup {
private:
    std::map<VkQueue, std::vector<VkCommandBuffer>> m_command_buffers;

public:
    CommandBufferGroup() = default;

    explicit CommandBufferGroup(const std::vector<Ptr<CommandBuffer>> &command_buffers);

    CommandBufferGroup(const std::initializer_list<Ptr<CommandBuffer>> &command_buffers);

    void Initialize(const std::vector<Ptr<CommandBuffer>> &command_buffers);

    VkResult Submit(const SemaphoreStageGroup &wait_semaphores = SemaphoreStageGroup(),
                    const SemaphoreGroup &signal_semaphores = SemaphoreGroup(),
                    const Ptr<Fence> &fence = nullptr) const;
};*/
} // namespace myvk

#endif

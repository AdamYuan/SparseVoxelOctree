#ifndef MYVK_COMMAND_BUFFER_HPP
#define MYVK_COMMAND_BUFFER_HPP

#include "DeviceObjectBase.hpp"
#include "Semaphore.hpp"
#include "Fence.hpp"
#include "CommandPool.hpp"
#include "BufferBase.hpp"
#include "Framebuffer.hpp"
#include "RenderPass.hpp"
#include "PipelineBase.hpp"
#include "DescriptorSet.hpp"

#include <memory>
#include <vector>
#include <map>
#include <volk.h>

namespace myvk {
	class CommandBuffer : public DeviceObjectBase {
	private:
		std::shared_ptr<CommandPool> m_command_pool_ptr;

		VkCommandBuffer m_command_buffer{nullptr};

	public:
		static std::shared_ptr<CommandBuffer>
		Create(const std::shared_ptr<CommandPool> &command_pool,
			   VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		static std::vector<std::shared_ptr<CommandBuffer>>
		CreateMultiple(const std::shared_ptr<CommandPool> &command_pool, uint32_t count,
					   VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		VkResult Submit(const SemaphoreStageGroup &wait_semaphores = SemaphoreStageGroup(),
						const SemaphoreGroup &signal_semaphores = SemaphoreGroup(),
						const std::shared_ptr<Fence> &fence = nullptr) const;

		VkResult Reset(VkCommandBufferResetFlags flags = 0) const;

		VkResult Begin(VkCommandBufferUsageFlags usage = 0) const;

		VkResult End() const;

		void
		CmdBeginRenderPass(const std::shared_ptr<RenderPass> &render_pass,
						   const std::shared_ptr<Framebuffer> &framebuffer,
						   const std::vector<VkClearValue> &clear_values, const VkExtent2D &extent,
						   const VkOffset2D &offset = {0, 0},
						   VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE) const;

		void CmdEndRenderPass() const;

		void CmdBindPipeline(const std::shared_ptr<PipelineBase> &pipeline) const;

		void CmdBindDescriptorSets(const std::vector<std::shared_ptr<DescriptorSet>> &descriptor_sets,
								   const std::shared_ptr<PipelineLayout> &pipeline_layout,
								   VkPipelineBindPoint pipeline_bind_point, const std::vector<uint32_t> &offsets = {}) const;

		void CmdBindDescriptorSets(const std::vector<std::shared_ptr<DescriptorSet>> &descriptor_sets,
								   const std::shared_ptr<PipelineBase> &pipeline,
								   const std::vector<uint32_t> &offsets = {}) const;

		void
		CmdBindVertexBuffer(const std::shared_ptr<BufferBase> &buffer, VkDeviceSize offset,
							uint32_t start_slot = 0) const;

		void
		CmdBindIndexBuffer(const std::shared_ptr<BufferBase> &buffer, VkDeviceSize offset,
						   VkIndexType type) const;

		void CmdSetViewport(const std::vector<VkViewport> &viewports) const;

		void CmdSetScissor(const std::vector<VkRect2D> &scissors) const;

		void CmdPushConstants(const std::shared_ptr<PipelineLayout> &pipeline_layout, VkShaderStageFlags shader_stage,
							  uint32_t offset, uint32_t size, const void *data) const;

		void CmdCopy(const std::shared_ptr<BufferBase> &src, const std::shared_ptr<BufferBase> &dst,
					 const std::vector<VkBufferCopy> &regions) const;

		void CmdCopy(const std::shared_ptr<BufferBase> &src, const std::shared_ptr<ImageBase> &dst,
					 const std::vector<VkBufferImageCopy> &regions,
					 VkImageLayout layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) const;

		void
		CmdDraw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) const;

		void CmdDrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, uint32_t vertex_offset,
							uint32_t first_instance) const;

		void CmdNextSubpass(VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE) const;

		void CmdPipelineBarrier(VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
								const std::vector<VkMemoryBarrier> &memory_barriers,
								const std::vector<VkBufferMemoryBarrier> &buffer_memory_barriers,
								const std::vector<VkImageMemoryBarrier> &image_memory_barriers) const;


		void CmdBlitImage(const std::shared_ptr<ImageBase> &src, const std::shared_ptr<ImageBase> &dst,
						  const VkImageBlit &blit, VkFilter filter) const;

		void CmdGenerateMipmap2D(const std::shared_ptr<ImageBase> &image,
								 VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
								 VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
								 VkImageLayout old_layout, VkImageLayout new_layout) const;

		void CmdDispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) const;

		void CmdDispatchIndirect(const std::shared_ptr<BufferBase> &buffer, VkDeviceSize offset = 0) const;

		VkCommandBuffer GetHandle() const { return m_command_buffer; }

		const std::shared_ptr<CommandPool> &GetCommandPoolPtr() const { return m_command_pool_ptr; }

		const std::shared_ptr<Device> &GetDevicePtr() const override { return m_command_pool_ptr->GetDevicePtr(); };

		~CommandBuffer();
	};

	class CommandBufferGroup {
	private:
		std::map<VkQueue, std::vector<VkCommandBuffer>> m_command_buffers;

	public:
		CommandBufferGroup() = default;

		explicit CommandBufferGroup(const std::vector<std::shared_ptr<CommandBuffer>> &command_buffers);

		CommandBufferGroup(const std::initializer_list<std::shared_ptr<CommandBuffer>> &command_buffers);

		void Initialize(const std::vector<std::shared_ptr<CommandBuffer>> &command_buffers);

		VkResult Submit(const SemaphoreStageGroup &wait_semaphores = SemaphoreStageGroup(),
						const SemaphoreGroup &signal_semaphores = SemaphoreGroup(),
						const std::shared_ptr<Fence> &fence = nullptr) const;
	};
} // namespace myvk

#endif

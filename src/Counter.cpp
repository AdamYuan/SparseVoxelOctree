#include "Counter.hpp"

void Counter::Initialize(const std::shared_ptr<myvk::Device> &device) {
	m_buffer = myvk::Buffer::Create(device, sizeof(uint32_t), VMA_MEMORY_USAGE_GPU_ONLY,
	                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
	                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	m_staging_buffer = myvk::Buffer::Create(device, sizeof(uint32_t), VMA_MEMORY_USAGE_CPU_ONLY,
	                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	m_fence = myvk::Fence::Create(device);
}

void Counter::Reset(const std::shared_ptr<myvk::CommandPool> &command_pool, uint32_t value) {
	std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(command_pool);
	m_staging_buffer->UpdateData((uint32_t)value, 0);
	command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	command_buffer->CmdCopy(m_staging_buffer, m_buffer, {{0, 0, sizeof(uint32_t)}});
	command_buffer->End();
	command_buffer->Submit(m_fence);
	m_fence->Wait();
	m_fence->Reset();
}

uint32_t Counter::Read(const std::shared_ptr<myvk::CommandPool> &command_pool) const {
	std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(command_pool);
	command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	command_buffer->CmdCopy(m_buffer, m_staging_buffer, {{0, 0, sizeof(uint32_t)}});
	command_buffer->End();
	command_buffer->Submit(m_fence);
	m_fence->Wait();
	m_fence->Reset();
	uint32_t ret = *((uint32_t *)m_staging_buffer->Map());
	m_staging_buffer->Unmap();
	return ret;
}

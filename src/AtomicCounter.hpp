#ifndef ATOMIC_COUNTER_HPP
#define ATOMIC_COUNTER_HPP

#include "myvk/Buffer.hpp"
#include "myvk/CommandBuffer.hpp"
#include "myvk/Fence.hpp"

class AtomicCounter {
private:
	std::shared_ptr<myvk::Buffer> m_buffer, m_staging_buffer;
	std::shared_ptr<myvk::Fence> m_fence;
public:
	void Initialize(const std::shared_ptr<myvk::Device> &device);
	void Reset(const std::shared_ptr<myvk::CommandPool> &command_pool, uint32_t value = 0);
	uint32_t Read(const std::shared_ptr<myvk::CommandPool> &command_pool) const;

	const std::shared_ptr<myvk::Buffer> &GetBufferPtr() const { return m_buffer; }
};


#endif

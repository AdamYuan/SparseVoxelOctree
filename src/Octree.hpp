#ifndef OCTREE_HPP
#define OCTREE_HPP

#include "myvk/Buffer.hpp"

class Octree {
private:
	std::shared_ptr<myvk::Buffer> m_buffer;
	uint32_t m_level{};
public:
	void Initialize(const std::shared_ptr<myvk::Buffer> &buffer, uint32_t level);

	const std::shared_ptr<myvk::Buffer> &GetBufferPtr() const { return m_buffer; }

	uint32_t GetLevel() const { return m_level; }
};


#endif

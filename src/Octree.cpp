#include "Octree.hpp"

void Octree::Initialize(const std::shared_ptr<myvk::Buffer> &buffer, uint32_t level) {
	m_buffer = buffer;
	m_level = level;
}

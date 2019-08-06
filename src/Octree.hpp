//
// Created by adamyuan on 19-5-18.
//

#ifndef SPARSEVOXELOCTREE_OCTREE_HPP
#define SPARSEVOXELOCTREE_OCTREE_HPP


#include <mygl3/shader.hpp>
#include "Voxelizer.hpp"

class Octree
{
private:
	mygl3::Shader m_tag_node_shader, m_alloc_node_shader;
	GLint m_unif_voxel_fragment_num, m_unif_voxel_resolution, m_unif_level, m_unif_alloc_begin, m_unif_alloc_num;
	mygl3::AtomicCounter m_counter;
	mygl3::Buffer m_octree;
	int m_octree_level;

	inline static GLuint group_x_64(unsigned x) { return (x >> 6u) + ((x & 0x3fu) ? 1u : 0u); }
public:
	void Initialize(int octree_level);
	void Build(const Voxelizer &voxelizer);
	const mygl3::Buffer &GetOctreeBuffer() const { return m_octree; }
	int GetLevel() const { return m_octree_level; }
};


#endif //SPARSEVOXELOCTREE_OCTREE_HPP

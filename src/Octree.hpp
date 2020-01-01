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
	mygl3::Buffer m_octree_buffer;
	int m_octree_level;
public:
	const mygl3::Buffer &GetOctreeBuffer() const { return m_octree_buffer; }
	int GetLevel() const { return m_octree_level; }

	friend class OctreeBuilder;
};


#endif //SPARSEVOXELOCTREE_OCTREE_HPP

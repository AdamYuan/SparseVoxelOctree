//
// Created by adamyuan on 2019/12/28.
//

#ifndef SPARSEVOXELOCTREE_OCTREEBUILDER_HPP
#define SPARSEVOXELOCTREE_OCTREEBUILDER_HPP

#include <mygl3/shader.hpp>
#include "Voxelizer.hpp"
#include "Octree.hpp"

class OctreeBuilder
{
private:
	mygl3::Shader m_tag_node_shader, m_alloc_node_shader, m_modify_arg_shader;
	mygl3::AtomicCounter m_counter;
	mygl3::Buffer m_alloc_indirect_buffer, m_build_info_buffer;
public:
	void Initialize();
	void Build(Octree *octree, const Voxelizer &voxelizer, int octree_level);
};


#endif //SPARSEVOXELOCTREE_OCTREEBUILDER_HPP

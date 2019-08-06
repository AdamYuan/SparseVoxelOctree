//
// Created by adamyuan on 19-5-4.
//

#ifndef SPARSEVOXELOCTREE_VOXELIZER_HPP
#define SPARSEVOXELOCTREE_VOXELIZER_HPP


#include <mygl3/shader.hpp>
#include <mygl3/atomiccounter.hpp>
#include <mygl3/framebuffer.hpp>
#include "Scene.hpp"

class Voxelizer
{
private:
	mygl3::Shader m_shader;
	mygl3::AtomicCounter m_counter;
	mygl3::Buffer m_fragment_list;
	mygl3::FrameBuffer m_fbo;
	mygl3::RenderBuffer m_rbo;
	GLint m_unif_voxel_resolution, m_unif_count_only;
	int m_fragment_num, m_resolution;

public:
	void Initialize(int octree_level);
	void Voxelize(const Scene &scene);
	const mygl3::Buffer &GetFragmentList() const { return m_fragment_list; }
	int GetFragmentNum() const { return m_fragment_num; }
};


#endif //SPARSEVOXELOCTREE_VOXELIZER_HPP

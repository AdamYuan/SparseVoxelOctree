//
// Created by adamyuan on 19-5-29.
//

#ifndef SPARSEVOXELOCTREE_OCTREETRACER_HPP
#define SPARSEVOXELOCTREE_OCTREETRACER_HPP


#include <mygl3/shader.hpp>
#include <mygl3/buffer.hpp>
#include <mygl3/vertexarray.hpp>
#include "Camera.hpp"
#include "Octree.hpp"

class OctreeTracer
{
public:
	enum ViewTypeEnum { kDiffuse = 0, kNormal, kIteration } m_view_type = kDiffuse;
	bool m_beam_enable{true};
	float m_beam_dir_size{0.01f}, m_beam_origin_size{0.002f};

private:
	mygl3::Shader m_shader, m_beam_shader;
	GLint m_unif_view_type, m_unif_beam_enable, m_beam_unif_dir_size, m_beam_unif_origin_size;
	mygl3::Buffer m_vbo; mygl3::VertexArray m_vao;
	mygl3::Texture2D m_beam_tex;
	mygl3::FrameBuffer m_beam_fbo;

public:
	void Initialize();
	void Render(const Octree &octree, const Camera &camera);
};


#endif //SPARSEVOXELOCTREE_OCTREETRACER_HPP

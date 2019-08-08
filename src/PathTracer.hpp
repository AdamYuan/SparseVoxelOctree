//
// Created by adamyuan on 19-8-8.
//

#ifndef SPARSEVOXELOCTREE_PATHTRACER_HPP
#define SPARSEVOXELOCTREE_PATHTRACER_HPP


#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>
#include <mygl3/buffer.hpp>

#include "Sobol.hpp"
#include "Camera.hpp"
#include "ScreenQuad.hpp"
#include "Octree.hpp"
#include "OctreeTracer.hpp"

class PathTracer
{
public:
	bool m_pause;
	int m_bounce{4};
	glm::vec3 m_sun_radiance{glm::vec3(5.0f)};
private:
	int m_spp;
	mygl3::Shader m_shader;
	GLint m_unif_beam_enable, m_unif_bounce, m_unif_spp, m_unif_sun_radiance;
	mygl3::Texture2D m_result_tex, m_noise_tex;
	mygl3::Buffer m_sobol_ssbo; GLfloat *m_sobol_ptr;
	Sobol m_sobol;
public:
	void Initialize();
	void Prepare(const Camera &camera, const Octree &octree, const OctreeTracer &tracer);
	void Render(const ScreenQuad &quad);
	void Save(const char *filename, bool fp16);
	int GetSPP() const { return m_spp; }
};


#endif //SPARSEVOXELOCTREE_PATHTRACER_HPP

//
// Created by adamyuan on 19-5-3.
//

#ifndef SPARSEVOXELOCTREE_RENDERER_HPP
#define SPARSEVOXELOCTREE_RENDERER_HPP

#include <mygl3/shader.hpp>
#include "Scene.hpp"
#include "Camera.hpp"

class Renderer
{
private:
	mygl3::Shader m_shader;
	GLint m_unif_projection, m_unif_view;
public:
	void Initialize();
	void Render(const Scene &scene, const Camera &camera);
};


#endif //SPARSEVOXELOCTREE_RENDERER_HPP

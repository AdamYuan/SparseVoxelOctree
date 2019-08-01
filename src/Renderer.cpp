//
// Created by adamyuan on 19-5-3.
//

#include "Renderer.hpp"
#include "OglBindings.hpp"
#include <glm/gtc/type_ptr.hpp>

void Renderer::Initialize()
{
	m_shader.Initialize();
	m_shader.LoadFromFile("shaders/view.frag", GL_FRAGMENT_SHADER);
	m_shader.LoadFromFile("shaders/view.vert", GL_VERTEX_SHADER);
	m_unif_view = m_shader.GetUniform("uView");
	m_unif_projection = m_shader.GetUniform("uProjection");
}

void Renderer::Render(const Scene &scene, const Camera &camera)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_CCW);
	m_shader.Use();
	camera.GetBuffer().BindBase(GL_UNIFORM_BUFFER, kCameraUBO);
	scene.Draw();
}

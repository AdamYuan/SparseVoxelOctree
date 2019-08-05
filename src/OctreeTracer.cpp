//
// Created by adamyuan on 19-5-29.
//
#include <glm/gtc/type_ptr.hpp>
#include "OctreeTracer.hpp"
#include "OglBindings.hpp"
#include "Config.hpp"

void OctreeTracer::Initialize()
{
	m_shader.Initialize();
	m_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_shader.LoadFromFile("shaders/octree_tracer.frag", GL_FRAGMENT_SHADER);

	m_beam_shader.Initialize();
	m_beam_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_beam_shader.LoadFromFile("shaders/octree_tracer_beam.frag", GL_FRAGMENT_SHADER);

	m_beam_tex.Initialize();
	m_beam_tex.Storage(kBeamWidth, kBeamHeight, GL_R32F);
	m_beam_tex.SetSizeFilter(GL_NEAREST, GL_NEAREST);
	m_beam_tex.SetWrapFilter(GL_CLAMP_TO_EDGE);

	m_beam_fbo.Initialize();
	m_beam_fbo.AttachTexture2D(m_beam_tex, GL_COLOR_ATTACHMENT0);

	constexpr GLfloat quad_vertices[] { -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
										1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f };
	m_vbo.Initialize();
	m_vbo.Storage(quad_vertices, quad_vertices + 12, 0);

	m_vao.Initialize();
	glEnableVertexArrayAttrib(m_vao.Get(), 0);
	glVertexArrayAttribFormat(m_vao.Get(), 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(m_vao.Get(), 0, 0);
	glVertexArrayVertexBuffer(m_vao.Get(), 0, m_vbo.Get(), 0, 2 * sizeof(GLfloat));
}

void OctreeTracer::Render(const Octree &octree, const Camera &camera)
{
	camera.GetBuffer().BindBase(GL_UNIFORM_BUFFER, kCameraUBO);
	octree.GetOctreeBuffer().BindBase(GL_SHADER_STORAGE_BUFFER, kOctreeSSBO);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	//perform beam optimization
	glViewport(0, 0, kBeamWidth, kBeamHeight);
	m_beam_fbo.Bind();
	m_beam_shader.Use();
	m_vao.Bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	mygl3::FrameBuffer::Unbind();

	//trace primary ray
	glViewport(0, 0, kWidth, kHeight);
	m_beam_tex.Bind(kBeamSampler2D);
	m_shader.Use();
	m_vao.Bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

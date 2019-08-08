//
// Created by adamyuan on 19-6-28.
//

#include "ScreenQuad.hpp"
#include "Config.hpp"

void ScreenQuad::Initialize()
{
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

void ScreenQuad::Render() const
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	m_vao.Bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

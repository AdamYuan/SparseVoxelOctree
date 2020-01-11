//
// Created by adamyuan on 19-5-29.
//
#include <glm/gtc/type_ptr.hpp>
#include "OctreeTracer.hpp"
#include "OglBindings.hpp"
#include "Config.hpp"
#include "ShaderSrc.hpp"

void OctreeTracer::Initialize()
{
	m_shader.Initialize();
	m_shader.Load(kQuadVertStr, GL_VERTEX_SHADER);
	m_shader.Load(kOctreeTracerFragStr, GL_FRAGMENT_SHADER);
	m_unif_view_type = m_shader.GetUniform("uViewType");
	m_unif_beam_enable = m_shader.GetUniform("uBeamEnable");

	m_beam_shader.Initialize();
	m_beam_shader.Load(kQuadVertStr, GL_VERTEX_SHADER);
	m_beam_shader.Load(kOctreeTracerBeamFragStr, GL_FRAGMENT_SHADER);
	m_beam_unif_dir_size = m_beam_shader.GetUniform("uDirSize");
	m_beam_unif_origin_size = m_beam_shader.GetUniform("uOriginSize");

	m_beam_tex.Initialize();
	m_beam_tex.Storage(kBeamWidth, kBeamHeight, GL_R32F);
	m_beam_tex.SetSizeFilter(GL_NEAREST, GL_NEAREST);
	m_beam_tex.SetWrapFilter(GL_CLAMP_TO_EDGE);

	m_beam_fbo.Initialize();
	m_beam_fbo.AttachTexture2D(m_beam_tex, GL_COLOR_ATTACHMENT0);
}

void OctreeTracer::Render(const ScreenQuad &quad, const Octree &octree, const Camera &camera)
{
	camera.GetBuffer().BindBase(GL_UNIFORM_BUFFER, kCameraUBO);
	octree.GetOctreeBuffer().BindBase(GL_SHADER_STORAGE_BUFFER, kOctreeSSBO);

	if(m_beam_enable)
	{
		//perform beam optimization
		glViewport(0, 0, kBeamWidth, kBeamHeight);
		m_beam_fbo.Bind();
		m_beam_shader.Use();
		m_beam_shader.SetFloat(m_beam_unif_dir_size, m_beam_dir_size);
		m_beam_shader.SetFloat(m_beam_unif_origin_size, m_beam_origin_size);
		quad.Render();
		mygl3::FrameBuffer::Unbind();
	}

	//trace primary ray
	glViewport(0, 0, kWidth, kHeight);
	m_beam_tex.Bind(kBeamSampler2D);
	m_shader.Use();
	m_shader.SetInt(m_unif_view_type, (GLint)m_view_type);
	m_shader.SetInt(m_unif_beam_enable, (GLint)m_beam_enable);
	quad.Render();
}

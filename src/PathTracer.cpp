//
// Created by adamyuan on 19-8-8.
//

#include <random>
#include <glm/gtc/type_ptr.hpp>
#include "PathTracer.hpp"
#include "Noise.inl"
#include "Config.hpp"
#include "OglBindings.hpp"
#include "Camera.hpp"
#include "Octree.hpp"
#include "OctreeTracer.hpp"
#include "ShaderSrc.hpp"

#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>

void PathTracer::Initialize()
{
	m_sobol_ssbo.Initialize();
	GLbitfield map_flags = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
	GLsizeiptr sobol_bytes = sizeof(GLfloat) * 2 * kMaxBounce;
	m_sobol_ssbo.Storage(sobol_bytes, map_flags);
	m_sobol_ptr = (GLfloat *)glMapNamedBufferRange(m_sobol_ssbo.Get(), 0, sobol_bytes, map_flags);

	m_noise_tex.Initialize();
	m_noise_tex.Storage(kNoiseSize, kNoiseSize, GL_RG8);
	m_noise_tex.Data(kNoise, kNoiseSize, kNoiseSize, GL_RG, GL_UNSIGNED_BYTE);

	m_pt_color_tex.Initialize();
	m_pt_color_tex.Storage(kWidth, kHeight, GL_RGBA32F);

	m_pt_albedo_tex.Initialize();
	m_pt_albedo_tex.Storage(kWidth, kHeight, GL_RGBA8);

	m_pt_normal_tex.Initialize();
	m_pt_normal_tex.Storage(kWidth, kHeight, GL_RGBA8_SNORM);
	//since normal in an svo only has 6 possibilities,
	//using RGBA8_SNORM to store its average is acceptable

	m_shader.Initialize();
	m_shader.Load(kQuadVertStr, GL_VERTEX_SHADER);
	m_shader.Load(kPathTracerFragStr, GL_FRAGMENT_SHADER);
	m_unif_view_type = m_shader.GetUniform("uViewType");
	m_unif_beam_enable = m_shader.GetUniform("uBeamEnable");
	m_unif_bounce = m_shader.GetUniform("uBounce");
	m_unif_spp = m_shader.GetUniform("uSPP");
	m_unif_sun_radiance = m_shader.GetUniform("uSunRadiance");
}

void PathTracer::Prepare(const Camera &camera, const Octree &octree, const OctreeTracer &tracer)
{
	//sobol initializationn
	m_sobol.Reset(m_bounce * 2);

	//bindings
	m_noise_tex.Bind(kNoiseSampler2D);
	m_sobol_ssbo.BindBase(GL_SHADER_STORAGE_BUFFER, kSobolSSBO);
	octree.GetOctreeBuffer().BindBase(GL_SHADER_STORAGE_BUFFER, kOctreeSSBO);
	camera.GetBuffer().BindBase(GL_UNIFORM_BUFFER, kCameraUBO);
	glBindImageTexture(kPTColorImage2D, m_pt_color_tex.Get(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(kPTAlbedoImage2D, m_pt_albedo_tex.Get(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
	glBindImageTexture(kPTNormalImage2D, m_pt_normal_tex.Get(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8_SNORM);

	if(tracer.m_beam_enable) tracer.m_beam_tex.Bind(kBeamSampler2D);

	//uniforms
	m_pause = false;
	m_spp = 0;
	m_shader.SetInt(m_unif_bounce, m_bounce);
	m_shader.SetInt(m_unif_beam_enable, (GLint) tracer.m_beam_enable);
	m_shader.SetVec3(m_unif_sun_radiance, glm::value_ptr(m_sun_radiance));
}

void PathTracer::Render(const ScreenQuad &quad)
{
	glViewport(0, 0, kWidth, kHeight);
	m_sobol.Next(m_sobol_ptr);
	m_shader.SetInt(m_unif_view_type, m_view_type);
	m_shader.SetInt(m_unif_spp, m_pause ? -1 : (m_spp ++));
	m_shader.Use();
	quad.Render();
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void PathTracer::Save(const char *filename, bool fp16)
{
	char *err{nullptr};

	constexpr int kSize = kWidth * kHeight;
	std::vector<GLfloat> pixels((size_t)kSize * 3);

	auto &target = (m_view_type == kColor ? m_pt_color_tex : (m_view_type == kAlbedo ? m_pt_albedo_tex : m_pt_normal_tex));
	glGetTextureImage(target.Get(), 0, GL_RGB, GL_FLOAT, kSize * 3 * sizeof(GLfloat), pixels.data());

	if(SaveEXR(pixels.data(), kWidth, kHeight, 3, fp16, filename, (const char **)&err) < 0)
		printf("[PT]ERR: %s\n", err);
	else
		printf("[PT]INFO: Saved image to %s\n", filename);

	free(err);
}

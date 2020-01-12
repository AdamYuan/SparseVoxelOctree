//
// Created by adamyuan on 19-5-4.
//

#include "Voxelizer.hpp"
#include "Config.hpp"
#include "OglBindings.hpp"
#include "ShaderSrc.hpp"

void Voxelizer::Initialize(int octree_level)
{
	m_resolution = 1 << octree_level;

	m_counter.Initialize();
	m_shader.Initialize();
	m_shader.Load(kVoxelizerFragStr, GL_FRAGMENT_SHADER);
	m_shader.Load(kVoxelizerVertStr, GL_VERTEX_SHADER);
	m_shader.Load(kVoxelizerGeomStr, GL_GEOMETRY_SHADER);
	m_unif_count_only = m_shader.GetUniform("uCountOnly");
	m_unif_voxel_resolution = m_shader.GetUniform("uVoxelResolution");
	m_shader.SetInt(m_unif_voxel_resolution, m_resolution);

	//Generate a 8x MSAA render buffer for MSAA Voxelization
	m_rbo.Initialize();
	glNamedRenderbufferStorageMultisample(m_rbo.Get(), 8, GL_R8, m_resolution, m_resolution);
	//m_rbo.Load(GL_R8, m_resolution, m_resolution);

	m_fbo.Initialize();
	m_fbo.AttachRenderbuffer(m_rbo, GL_COLOR_ATTACHMENT0);
}

void Voxelizer::Voxelize(const Scene &scene)
{
	m_fbo.Bind();
	glViewport(0, 0, m_resolution, m_resolution);

	m_counter.BindAtomicCounter(kAtomicCounterBinding);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	m_shader.Use();

	//fragment list not initialized, first count the voxels
	if(!mygl3::IsValidOglId(m_fragment_list.Get()))
	{
		m_counter.Reset();
		m_shader.SetInt(m_unif_count_only, 1);
		scene.Draw();

		m_fragment_num = m_counter.SyncAndGetValue();
		m_fragment_list.Initialize();
		m_fragment_list.Storage(m_fragment_num * sizeof(GLuint) * 2, 0);
		printf("[VOXELIZER]Info: Created fragment buffer with %d voxels, %.1lf MB\n", m_fragment_num,
			   m_fragment_list.GetByteCount() / 1000000.0);
	}

	GLuint time_query; glGenQueries(1, &time_query);
	glBeginQuery(GL_TIME_ELAPSED, time_query);

	m_counter.Reset();
	m_fragment_list.BindBase(GL_SHADER_STORAGE_BUFFER, kVoxelFragmentListSSBO);
	m_shader.SetInt(m_unif_count_only, 0);
	scene.Draw();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

	glEndQuery(GL_TIME_ELAPSED);

	GLint done = 0; while(!done)
		glGetQueryObjectiv(time_query, GL_QUERY_RESULT_AVAILABLE, &done);
	GLuint64 time_elapsed;
	glGetQueryObjectui64v(time_query, GL_QUERY_RESULT, &time_elapsed);
	glDeleteQueries(1, &time_query);

	printf("[VOXELIZER]Info: Fragment buffer filled, in %f ms.\n",
			time_elapsed / 1000000.0);

	mygl3::FrameBuffer::Unbind();
	glViewport(0, 0, kWidth, kHeight);
}


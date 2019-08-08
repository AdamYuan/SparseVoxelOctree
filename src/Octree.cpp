//
// Created by adamyuan on 19-5-18.
//

#include "Octree.hpp"
#include "Config.hpp"
#include "OglBindings.hpp"

void Octree::Initialize(int octree_level)
{
	m_octree_level = octree_level;

	m_tag_node_shader.Initialize();
	m_tag_node_shader.LoadFromFile("shaders/octree_tag_node.comp", GL_COMPUTE_SHADER);
	m_unif_voxel_resolution = m_tag_node_shader.GetUniform("uVoxelResolution");
	m_unif_level = m_tag_node_shader.GetUniform("uLevel");
	m_unif_voxel_fragment_num = m_tag_node_shader.GetUniform("uVoxelFragmentNum");
	m_tag_node_shader.SetInt(m_unif_voxel_resolution, 1 << m_octree_level);

	m_alloc_node_shader.Initialize();
	m_alloc_node_shader.LoadFromFile("shaders/octree_alloc_node.comp", GL_COMPUTE_SHADER);
	m_unif_alloc_begin = m_alloc_node_shader.GetUniform("uAllocBegin");
	m_unif_alloc_num = m_alloc_node_shader.GetUniform("uAllocNum");

	m_counter.Initialize();
}

void Octree::Build(const Voxelizer &voxelizer)
{

	//estimate octree buffer size and initialize octree buffer
	int octree_node_num = std::max(kOctreeNodeNumMin, voxelizer.GetFragmentNum() << 2);
	octree_node_num = std::min(octree_node_num, kOctreeNodeNumMax);
	m_octree.Initialize();
	m_octree.Storage(octree_node_num*sizeof(GLuint), GL_MAP_WRITE_BIT);
	printf("[OCTREE]Info: Alloc %d nodes, %.1fMB\n",
		   octree_node_num, octree_node_num*sizeof(GLuint) / 1000000.0f);

	//clear the buffer
	GLuint *octree_map;
	octree_map = (GLuint *)
			glMapNamedBufferRange(m_octree.Get(), 0, octree_node_num*sizeof(GLuint), GL_MAP_WRITE_BIT);
	std::fill(octree_map, octree_map + octree_node_num, 0u);
	glUnmapNamedBuffer(m_octree.Get());

	//reset atomic counter
	m_counter.Reset();

	//bind buffers
	voxelizer.GetFragmentList().BindBase(GL_SHADER_STORAGE_BUFFER, kVoxelFragmentListSSBO);
	m_octree.BindBase(GL_SHADER_STORAGE_BUFFER, kOctreeSSBO);
	m_counter.BindAtomicCounter(kAtomicCounterBinding);

	//set static uniforms
	m_tag_node_shader.SetInt(m_unif_voxel_fragment_num, voxelizer.GetFragmentNum());

	GLuint frag_list_group_x = group_x_64(voxelizer.GetFragmentNum());

	GLuint last_count = 0, alloc_num = 8, alloc_begin = 0;
	for(int cur = 1; cur <= m_octree_level; ++cur)
	{
		m_tag_node_shader.Use();
		m_tag_node_shader.SetInt(m_unif_level, cur);
		glDispatchCompute(frag_list_group_x, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		if(cur != m_octree_level) //the last octree level doesn't has leaves
		{
			m_alloc_node_shader.Use();
			m_alloc_node_shader.SetInt(m_unif_alloc_begin, alloc_begin);
			m_alloc_node_shader.SetInt(m_unif_alloc_num, alloc_num);
			glDispatchCompute(group_x_64(alloc_num), 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			mygl3::SyncGPU();

			alloc_begin += alloc_num;
			alloc_num = (m_counter.GetValue() - last_count) << 3u;
			last_count = m_counter.GetValue();
		}

		printf("[OCTREE]Info: Level %d built\n", cur);
	}

	printf("[OCTREE]Info: Building complete, used %d nodes\n", alloc_begin + alloc_num);
}

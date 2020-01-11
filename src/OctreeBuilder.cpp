//
// Created by adamyuan on 2019/12/28.
//

#include "OctreeBuilder.hpp"
#include "Config.hpp"
#include "OglBindings.hpp"
#include "ShaderSrc.hpp"

struct DispatchIndirectCommand {
	GLuint num_groups_x, num_groups_y, num_groups_z;
};
struct BuildInfo {
	GLuint fragment_num, voxel_resolution, alloc_begin, alloc_num;
};

inline static GLuint group_x_64(unsigned x) { return (x >> 6u) + ((x & 0x3fu) ? 1u : 0u); }

void OctreeBuilder::Initialize()
{
	m_tag_node_shader.Initialize();
	m_tag_node_shader.Load(kOctreeTagNodeCompStr, GL_COMPUTE_SHADER);

	m_alloc_node_shader.Initialize();
	m_alloc_node_shader.Load(kOctreeAllocNodeCompStr, GL_COMPUTE_SHADER);

	m_modify_arg_shader.Initialize();
	m_modify_arg_shader.Load(kOctreeModifyArgCompStr, GL_COMPUTE_SHADER);

	m_counter.Initialize();

	m_alloc_indirect_buffer.Initialize();
	m_alloc_indirect_buffer.Storage(sizeof(DispatchIndirectCommand), GL_MAP_WRITE_BIT);

	m_build_info_buffer.Initialize();
	m_build_info_buffer.Storage(sizeof(BuildInfo), GL_MAP_WRITE_BIT);
}

void OctreeBuilder::Build(Octree *octree, const Voxelizer &voxelizer, int octree_level)
{
	//set properties
	octree->m_octree_level = octree_level;

	//set alloc node indirect buffer
	DispatchIndirectCommand *cmd_map;
	cmd_map = (DispatchIndirectCommand *)
			glMapNamedBufferRange(m_alloc_indirect_buffer.Get(), 0,
								  sizeof(DispatchIndirectCommand), GL_MAP_WRITE_BIT);
	cmd_map->num_groups_x = 0;
	cmd_map->num_groups_y = cmd_map->num_groups_z = 1;
	glUnmapNamedBuffer(m_alloc_indirect_buffer.Get());

	//set initial info buffer
	BuildInfo *info_map;
	info_map = (BuildInfo *)
			glMapNamedBufferRange(m_build_info_buffer.Get(), 0,
								  sizeof(BuildInfo), GL_MAP_WRITE_BIT);
	info_map->fragment_num = voxelizer.GetFragmentNum();
	info_map->voxel_resolution = 1u << (GLuint)octree_level;
	info_map->alloc_begin = info_map->alloc_num = 0;
	glUnmapNamedBuffer(m_build_info_buffer.Get());

	//estimate octree buffer size and initialize octree buffer
	int octree_node_num = std::max(kOctreeNodeNumMin, voxelizer.GetFragmentNum() << 2);
	octree_node_num = std::min(octree_node_num, kOctreeNodeNumMax);
	octree->m_octree_buffer.Initialize();
	octree->m_octree_buffer.Storage(octree_node_num * sizeof(GLuint), GL_MAP_WRITE_BIT);

	printf("[OCTREE]Info: Alloc %d nodes, %.1fMB\n",
		   octree_node_num, octree->m_octree_buffer.GetByteCount() / 1000000.0f);

	//clear the octree buffer
	GLuint *octree_map;
	octree_map = (GLuint *)
			glMapNamedBufferRange(octree->m_octree_buffer.Get(), 0, octree_node_num*sizeof(GLuint), GL_MAP_WRITE_BIT);
	std::fill(octree_map, octree_map + octree_node_num, 0u);
	glUnmapNamedBuffer(octree->m_octree_buffer.Get());

	//reset atomic counter
	m_counter.Reset();

	//bind buffers
	voxelizer.GetFragmentList().BindBase(GL_SHADER_STORAGE_BUFFER, kVoxelFragmentListSSBO);
	octree->m_octree_buffer.BindBase(GL_SHADER_STORAGE_BUFFER, kOctreeSSBO);
	m_alloc_indirect_buffer.BindBase(GL_SHADER_STORAGE_BUFFER, kOctreeAllocIndirectSSBO);
	m_alloc_indirect_buffer.Bind(GL_DISPATCH_INDIRECT_BUFFER);
	m_build_info_buffer.BindBase(GL_SHADER_STORAGE_BUFFER, kOctreeBuildInfoSSBO);
	m_counter.BindAtomicCounter(kAtomicCounterBinding);

	GLuint frag_list_group_x = group_x_64(voxelizer.GetFragmentNum());

	GLuint time_query; glGenQueries(1, &time_query);
	glBeginQuery(GL_TIME_ELAPSED, time_query);
	for(int cur = 1; cur <= octree_level; ++cur)
	{
		m_tag_node_shader.Use();
		glDispatchCompute(frag_list_group_x, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		if(cur != octree_level) //the last octree level doesn't has leaves
		{
			m_modify_arg_shader.Use();
			glDispatchCompute(1, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BUFFER | GL_COMMAND_BARRIER_BIT);

			m_alloc_node_shader.Use();
			glDispatchComputeIndirect(0);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
		}
	}
	glEndQuery(GL_TIME_ELAPSED);

	GLint done = 0; while(!done)
		glGetQueryObjectiv(time_query, GL_QUERY_RESULT_AVAILABLE, &done);
	GLuint64 time_elapsed;
	glGetQueryObjectui64v(time_query, GL_QUERY_RESULT, &time_elapsed);
	glDeleteQueries(1, &time_query);

	printf("[OCTREE]Info: Building complete, in %f ms.\n",
		   time_elapsed / 1000000.0);
}

//
// Created by adamyuan on 19-5-3.
//

#ifndef SPARSEVOXELOCTREE_SCENE_HPP
#define SPARSEVOXELOCTREE_SCENE_HPP

#include <vector>
#include <string>
#include <mygl3/texture.hpp>
#include <mygl3/buffer.hpp>
#include <mygl3/vertexarray.hpp>

class Scene //with all the vertex normalized to [-1, 1]
{
private:
	std::vector<mygl3::Texture2D> m_textures;
	mygl3::Buffer m_vertex_buffer, m_material_buffer, m_draw_indirect_buffer;
	mygl3::Buffer m_texture_handle_buffer; //for bindless texture
	mygl3::VertexArray m_vao;

	int m_draw_cnt;

public:
	bool Initialize(const char *filename);
	void Draw() const;
};


#endif //SPARSEVOXELOCTREE_SCENE_HPP

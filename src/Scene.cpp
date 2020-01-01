//
// Created by adamyuan on 19-5-3.
//

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>
#include <glm/glm.hpp>

#include "Scene.hpp"
#include "OglBindings.hpp"
#include <cstring>

struct Vertex {
	glm::vec3 position, normal;
	glm::vec2 texcoord;
};
struct Mesh {
	std::vector<Vertex> vertices;
};
struct DrawArraysIndirectCommand {
	GLuint vertex_count, instance_count, vertex_base, instance_base;
} ;
struct Material {
	GLuint texture;
	glm::vec3 color;
};

static std::string get_base_dir(const char *filename)
{
	size_t len = strlen(filename);
	if(len == 0)
	{
		printf("[SCENE]Filename invalid\n");
		return {};
	}
	const char *s = filename + len;
	while(*(s - 1) != '/' && *(s - 1) != '\\' && s > filename) s --;
	return {filename, s};
}

static int load_texture(std::vector<mygl3::Texture2D> *textures,
						std::map<std::string, int> &name_set,
						const char *filename)
{
	if(name_set.count(filename)) return name_set[filename];

	textures->emplace_back();

	mygl3::Texture2D &cur = textures->back();
	cur.Initialize();

	GLsizei width, height, channels;
	GLvoid *data{};

	data = stbi_load(filename, &width, &height, &channels, 4);
	if(data == nullptr)
	{
		printf("[SCENE]Err: Unable to load texture %s\n", filename);
		textures->pop_back();
		return -1;
	}
	cur.Storage(width, height, GL_RGBA8, mygl3::Texture2D::GetLevelCount(width, height));
	cur.Data(data, width, height, GL_RGBA, GL_UNSIGNED_BYTE);
	stbi_image_free(data);

	cur.GenerateMipmap();
	cur.SetWrapFilter(GL_REPEAT);
	cur.SetSizeFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	printf("[SCENE]Info: Loaded texture %s\n", filename);

	int idx = (int)textures->size() - 1;
	return name_set[filename] = idx;
}

static bool load_obj(const char *filename, const std::string &base_dir,
					 std::vector<tinyobj::material_t> *materials,
					 std::vector<Mesh> *meshes)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;

	std::string err;
	if(!tinyobj::LoadObj(&attrib, &shapes, materials, &err, filename, base_dir.c_str()))
	{
		printf("[SCENE]Failed to load %s\n", filename);
		return false;
	}

	if(materials->size() == 0)
	{
		printf("[SCENE]No material found\n");
		return false;
	}

	if (!err.empty())
		printf("%s\n", err.c_str());

	meshes->resize(materials->size());

	glm::vec3 pmin(FLT_MAX), pmax(-FLT_MAX);

	// Loop over shapes
	for(const auto &shape : shapes)
	{
		size_t index_offset = 0, face = 0;

		// Loop over faces(polygon)
		for(const auto &num_face_vertex : shape.mesh.num_face_vertices)
		{
			int mat = shape.mesh.material_ids[face];
			// Loop over triangles in the face.
			for (size_t v = 0; v < num_face_vertex; ++v)
			{
				tinyobj::index_t index = shape.mesh.indices[index_offset + v];

				meshes->at(mat).vertices.emplace_back();
				Vertex &vert = meshes->at(mat).vertices.back();
				vert.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
				};
				pmin = glm::min(vert.position, pmin);
				pmax = glm::max(vert.position, pmax);
				if(~index.normal_index)
					vert.normal = {
							attrib.normals[3 * index.normal_index + 0],
							attrib.normals[3 * index.normal_index + 1],
							attrib.normals[3 * index.normal_index + 2]
					};

				if(~index.texcoord_index)
					vert.texcoord = {
							attrib.texcoords[2 * index.texcoord_index + 0],
							1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};
			}
			index_offset += num_face_vertex;
			face ++;
		}
	}


	//normalize all the vertex to [-1, 1]
	glm::vec3 extent3 = pmax - pmin;
	float extent = glm::max(extent3.x, glm::max(extent3.y, extent3.z)) * 0.5f;
	float inv_extent = 1.0f / extent;

	glm::vec3 center = (pmax + pmin) * 0.5f;

	for(auto &m : *meshes)
		for(auto &v : m.vertices)
			v.position = (v.position - center) * inv_extent;

	printf("[SCENE]Info: %ld meshes loaded from %s\n", meshes->size(), filename);
	return true;
}

bool Scene::Initialize(const char *filename)
{
	std::string base_dir = get_base_dir(filename);

	//these 2 have the same size
	std::vector<Mesh> meshes;
	std::vector<tinyobj::material_t> materials;

	if(!load_obj(filename, base_dir, &materials, &meshes)) return false;

	m_draw_cnt = meshes.size();

	//load materials
	//load all the textures
	std::vector<Material> gpu_materials(m_draw_cnt);
	std::map<std::string, int> name_set;
	for(size_t i = 0; i < m_draw_cnt; ++i)
	{
		Material &cur = gpu_materials[i];
		if(materials[i].diffuse_texname.empty())
			cur.texture = 0xffffffffu;
		else
			cur.texture = load_texture(&m_textures, name_set,
									   (base_dir + materials[i].diffuse_texname).c_str());

		cur.color.x = materials[i].diffuse[0];
		cur.color.y = materials[i].diffuse[1];
		cur.color.z = materials[i].diffuse[2];
	}

	//process the indirect draw commands
	std::vector<DrawArraysIndirectCommand> commands(m_draw_cnt);
	size_t vert_cnt = 0;
	for(size_t i = 0; i < m_draw_cnt; ++i)
	{
		commands[i].vertex_count = meshes[i].vertices.size();
		commands[i].vertex_base = vert_cnt;

		commands[i].instance_count = 1;
		commands[i].instance_base = i;

		vert_cnt += commands[i].vertex_count;
	}

	//copy all the vertices to a big buffer
	std::vector<Vertex> vertices(vert_cnt);
	for(size_t i = 0; i < m_draw_cnt; ++i)
	{
		std::copy(meshes[i].vertices.data(),
				  meshes[i].vertices.data() + commands[i].vertex_count,
				  vertices.data() + commands[i].vertex_base);
		meshes[i].vertices.clear();
		meshes[i].vertices.shrink_to_fit();
	}

	//process indirect textures
	std::vector<GLuint64> texture_handles;
	for(const auto &t : m_textures)
	{
		GLuint64 handle = glGetTextureHandleARB(t.Get());
		texture_handles.push_back(handle);
		glMakeTextureHandleResidentARB(handle);
	}
	m_texture_handle_buffer.Initialize();
	m_texture_handle_buffer.Storage(texture_handles.data(), texture_handles.data() + texture_handles.size(), 0);

	m_draw_indirect_buffer.Initialize();
	m_draw_indirect_buffer.Storage(commands.data(), commands.data() + commands.size(), 0);

	m_vertex_buffer.Initialize();
	m_vertex_buffer.Storage(vertices.data(), vertices.data() + vertices.size(), 0);

	m_material_buffer.Initialize();
	m_material_buffer.Storage(gpu_materials.data(), gpu_materials.data() + gpu_materials.size(), 0);

	//0-position, 1-normal, 2-texcoord, 3-drawid
	m_vao.Initialize();
	{
		constexpr GLuint kPos = 0, kNormal = 1, kTexcoord = 2, kTexture = 3, kColor = 4;
		constexpr GLuint kVbo0 = 0, kVbo1 = 1;
		glEnableVertexArrayAttrib(m_vao.Get(), kPos);
		glVertexArrayAttribFormat(m_vao.Get(), kPos, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_vao.Get(), kPos, kVbo0);

		glEnableVertexArrayAttrib(m_vao.Get(), kNormal);
		glVertexArrayAttribFormat(m_vao.Get(), kNormal, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vao.Get(), kNormal, kVbo0);

		glEnableVertexArrayAttrib(m_vao.Get(), kTexcoord);
		glVertexArrayAttribFormat(m_vao.Get(), kTexcoord, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vao.Get(), kTexcoord, kVbo0);

		glVertexArrayBindingDivisor(m_vao.Get(), kVbo1, 1);

		glEnableVertexArrayAttrib(m_vao.Get(), kTexture);
		glVertexArrayAttribIFormat(m_vao.Get(), kTexture, 1, GL_UNSIGNED_INT, 0);
		glVertexArrayAttribBinding(m_vao.Get(), kTexture, kVbo1);

		glEnableVertexArrayAttrib(m_vao.Get(), kColor);
		glVertexArrayAttribFormat(m_vao.Get(), kColor, 3, GL_FLOAT, GL_TRUE, sizeof(GLuint));
		glVertexArrayAttribBinding(m_vao.Get(), kColor, kVbo1);
	}

	GLuint buffers[2] = {m_vertex_buffer.Get(), m_material_buffer.Get()};
	GLintptr offsets[2] = {0, 0};
	GLsizei strides[2] = {8 * sizeof(GLfloat), sizeof(GLuint) + sizeof(GLfloat) * 3};
	glVertexArrayVertexBuffers(m_vao.Get(), 0, 2, buffers, offsets, strides);

	m_texture_handle_buffer.BindBase(GL_UNIFORM_BUFFER, kTextureUBO);
	return true;
}

void Scene::Draw() const
{
	m_vao.Bind();
	m_draw_indirect_buffer.Bind(GL_DRAW_INDIRECT_BUFFER);
	glMultiDrawArraysIndirect(GL_TRIANGLES, nullptr, m_draw_cnt, 0);
}


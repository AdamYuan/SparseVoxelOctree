#include "Scene.hpp"
#include "myvk/ObjectTracker.hpp"

#include <string>
#include <unordered_map>
#include <thread>
#include <future>
#include <mutex>
#include <algorithm>

#include <tiny_obj_loader.h>
#include <stb_image.h>

#include <plog/Log.h>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/hash.hpp>

struct Vertex {
	glm::vec3 m_position;
	glm::vec3 m_normal;
	glm::vec2 m_texcoord;

	bool operator==(const Vertex &r) const {
		return m_position == r.m_position && m_normal == r.m_normal && m_texcoord == r.m_texcoord;
	}
};
namespace std {
	template<>
	struct hash<Vertex> {
		size_t operator()(const Vertex &vert) const {
			return ((hash<glm::vec3>()(vert.m_position) ^ (hash<glm::vec3>()(vert.m_normal) << 1u)) >> 1u) ^
				   (hash<glm::vec2>()(vert.m_texcoord) << 1u);
		}
	};
}
struct Scene::Mesh {
	uint32_t m_texture_id;
	glm::vec3 m_albedo;
	std::vector<Vertex> m_vertices;
};

static std::string get_base_dir(const char *filename) {
	size_t len = strlen(filename);
	if (len == 0) {
		LOGE.printf("[SCENE]Filename \"%s\" invalid", filename);
		return {};
	}
	const char *s = filename + len;
	while (*(s - 1) != '/' && *(s - 1) != '\\' && s > filename) s--;
	return {filename, s};
}

bool Scene::load_meshes(const char *filename, const char *base_dir, std::vector<Mesh> *meshes,
						std::vector<std::string> *texture_filenames) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::material_t> materials;
	std::vector<tinyobj::shape_t> shapes;

	std::string load_outputs;
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &load_outputs, filename, base_dir)) {
		LOGE.printf("Failed to load \"%s\"", filename);
		return false;
	}
	if (materials.empty()) {
		LOGE.printf("No material found");
		return false;
	}
	if (!load_outputs.empty()) {
		LOGW.printf("%s", load_outputs.c_str());
	}

	meshes->clear();
	meshes->resize(materials.size());

	glm::vec3 pmin(FLT_MAX), pmax(-FLT_MAX);
	// Loop over shapes
	for (const auto &shape : shapes) {
		size_t index_offset = 0, face = 0;
		// Loop over faces(polygon)
		for (const auto &num_face_vertex : shape.mesh.num_face_vertices) {
			int mat = shape.mesh.material_ids[face];
			// Loop over triangles in the face.
			for (size_t v = 0; v < num_face_vertex; ++v) {
				tinyobj::index_t index = shape.mesh.indices[index_offset + v];

				meshes->at(mat).m_vertices.emplace_back();
				Vertex &vert = meshes->at(mat).m_vertices.back();
				vert.m_position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
				pmin = glm::min(vert.m_position, pmin);
				pmax = glm::max(vert.m_position, pmax);
				if (index.normal_index != -1)
					vert.m_normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};

				if (index.texcoord_index != -1)
					vert.m_texcoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};
			}
			index_offset += num_face_vertex;
			face++;
		}
	}

	//normalize all the vertex to [-1, 1]
	{
		glm::vec3 extent3 = pmax - pmin;
		float extent = glm::max(extent3.x, glm::max(extent3.y, extent3.z)) * 0.5f;
		float inv_extent = 1.0f / extent;
		glm::vec3 center = (pmax + pmin) * 0.5f;
		for (auto &i : *meshes)
			for (auto &v : i.m_vertices)
				v.m_position = (v.m_position - center) * inv_extent;
	}

	//set mesh material
	std::unordered_map<std::string, uint32_t> texture_name_map;
	for (uint32_t i = 0; i < meshes->size(); ++i) {
		Mesh &mesh = (*meshes)[i];
		if (mesh.m_vertices.empty()) continue;
		mesh.m_albedo = {
			materials[i].diffuse[0],
			materials[i].diffuse[1],
			materials[i].diffuse[2]
		};
		const std::string &texture_name = materials[i].diffuse_texname;
		if (!texture_name.empty() && texture_name_map.find(texture_name) == texture_name_map.end())
			texture_name_map[texture_name] = texture_name_map.size();
		else if (texture_name.empty())
			mesh.m_texture_id = UINT32_MAX;
	}

	//set mesh texture id
	for (uint32_t i = 0; i < meshes->size(); ++i) {
		Mesh &mesh = (*meshes)[i];
		if (!mesh.m_vertices.empty() && mesh.m_texture_id != UINT32_MAX) {
			mesh.m_texture_id = texture_name_map[materials[i].diffuse_texname];
		}
	}

	//cull empty meshes
	{
		uint32_t not_empty_cnt = meshes->size();
		for (const Mesh &i : *meshes) not_empty_cnt -= i.m_vertices.empty();
		std::sort(meshes->begin(), meshes->end(), [](const Mesh &l, const Mesh &r) {
			return l.m_vertices.size() > r.m_vertices.size();
		});
		meshes->resize(not_empty_cnt);
	}

	if (meshes->empty()) {
		LOGE.printf("Empty mesh");
		return false;
	}

	texture_filenames->resize(texture_name_map.size());
	for (auto &i : texture_name_map)
		(*texture_filenames)[i.second] = base_dir + i.first;

	return true;
}

void
Scene::load_buffers_and_draw_cmd(const std::shared_ptr<myvk::Queue> &graphics_queue, const std::vector<Mesh> &meshes) {
	m_draw_commands.clear();
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	{
		//shrink vertices
		uint32_t vertex_cnt = 0;
		std::unordered_map<Vertex, uint32_t> vertex_map;
		for (const Mesh &mesh : meshes) {
			for (const Vertex &vertex : mesh.m_vertices) {
				if (vertex_map.find(vertex) == vertex_map.end())
					vertex_map[vertex] = vertex_map.size();
			}
			vertex_cnt += mesh.m_vertices.size();
		}
		LOGI.printf("%u/%u vertices after shrinking", vertex_map.size(), vertex_cnt);

		//fill vertex/index buffers and draw commands
		vertices.resize(vertex_map.size());
		for (const auto &i : vertex_map)
			vertices[i.second] = i.first;
		indices.resize(vertex_cnt);
		m_draw_commands.resize(meshes.size());
		for (uint32_t i = 0, c = 0; i < meshes.size(); ++i) {
			const Mesh &mesh = meshes[i];
			m_draw_commands[i].m_push_constant.m_texture_id = mesh.m_texture_id;
			m_draw_commands[i].m_push_constant.m_albedo = glm::packUnorm4x8(glm::vec4(mesh.m_albedo, 0.0f));
			m_draw_commands[i].m_first_index = c;
			m_draw_commands[i].m_index_count = mesh.m_vertices.size();

			for (uint32_t j = 0; j < mesh.m_vertices.size(); ++j)
				indices[c + j] = vertex_map[mesh.m_vertices[j]];

			c += mesh.m_vertices.size();
		}
	}

	const std::shared_ptr<myvk::Device> &device = graphics_queue->GetDevicePtr();
	uint32_t
		vertex_buffer_size = vertices.size() * sizeof(Vertex),
		index_buffer_size = indices.size() * sizeof(uint32_t);
	m_vertex_buffer = myvk::Buffer::Create(device, vertex_buffer_size, VMA_MEMORY_USAGE_GPU_ONLY,
										   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	m_index_buffer = myvk::Buffer::Create(device, index_buffer_size, VMA_MEMORY_USAGE_GPU_ONLY,
										  VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	std::shared_ptr<myvk::Buffer> vertex_staging_buffer = myvk::Buffer::CreateStaging(device, vertex_buffer_size);
	vertex_staging_buffer->UpdateData(vertices.data(), vertices.data() + vertices.size());
	std::shared_ptr<myvk::Buffer> index_staging_buffer = myvk::Buffer::CreateStaging(device, index_buffer_size);
	index_staging_buffer->UpdateData(indices.data(), indices.data() + indices.size());

	std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(device);
	std::shared_ptr<myvk::CommandPool> command_pool = myvk::CommandPool::Create(graphics_queue);
	std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(command_pool);
	command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	command_buffer->CmdCopy(vertex_staging_buffer, m_vertex_buffer, {{0, 0, vertex_buffer_size}});
	command_buffer->CmdCopy(index_staging_buffer, m_index_buffer, {{0, 0, index_buffer_size}});
	command_buffer->End();
	command_buffer->Submit({}, {}, fence);

	fence->Wait();

	LOGI.printf("Vertex and Index buffers generated");
}

void Scene::load_textures(const std::shared_ptr<myvk::Queue> &graphics_queue,
						  const std::vector<std::string> &texture_filenames) {
	const std::shared_ptr<myvk::Device> &device = graphics_queue->GetDevicePtr();
	m_textures.clear();
	m_textures.resize(texture_filenames.size());

	//multi-threaded texture loading
	unsigned cores = std::thread::hardware_concurrency();
	std::vector<std::future<void>> future_vector;
	future_vector.reserve(cores);
	std::atomic_uint32_t texture_id{0};
	std::mutex mutex;
	while (cores--) {
		future_vector.push_back(std::async(
			[&]() -> void {
				std::shared_ptr<myvk::CommandPool> command_pool = myvk::CommandPool::Create(graphics_queue,
																							VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
				myvk::ObjectTracker tracker;
				while (true) {
					uint32_t i = texture_id++;
					if (i >= texture_filenames.size())
						break;

					//Load texture data from file
					int width, height, channels;
					stbi_uc *data = stbi_load(texture_filenames[i].c_str(), &width, &height, &channels, 4);
					if (data == nullptr) {
						LOGE.printf("Unable to load texture %s", texture_filenames[i].c_str());
						continue;
					}
					uint32_t data_size = width * height * 4;
					VkExtent2D extent = {(uint32_t) width, (uint32_t) height};
					//Create staging buffer
					std::shared_ptr<myvk::Buffer> staging_buffer = myvk::Buffer::CreateStaging(device, data_size);
					staging_buffer->UpdateData(data, data + data_size);
					//Free texture data
					stbi_image_free(data);

					Texture &texture = m_textures[i];
					//Create image
					texture.m_image = myvk::Image::CreateTexture2D(device, extent, myvk::Image::QueryMipLevel(extent),
																   VK_FORMAT_R8G8B8A8_SRGB,
																   VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
																   VK_IMAGE_USAGE_TRANSFER_DST_BIT |
																   VK_IMAGE_USAGE_SAMPLED_BIT);
					//Create ImageView and Sampler
					texture.m_image_view = myvk::ImageView::Create(texture.m_image, VK_IMAGE_VIEW_TYPE_2D);
					texture.m_sampler = myvk::Sampler::Create(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
															  VK_SAMPLER_MIPMAP_MODE_LINEAR,
															  texture.m_image->GetMipLevels());

					//Copy buffer to image and generate mipmap
					std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(device);
					std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(command_pool);
					command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
					VkBufferImageCopy region = {};
					region.bufferOffset = 0;
					region.bufferRowLength = 0;
					region.bufferImageHeight = 0;
					region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					region.imageSubresource.mipLevel = 0;
					region.imageSubresource.baseArrayLayer = 0;
					region.imageSubresource.layerCount = 1;
					region.imageOffset = {0, 0, 0};
					region.imageExtent = {(uint32_t) width, (uint32_t) height, 1};

					command_buffer->CmdPipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
													   VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {},
													   texture.m_image->GetDstMemoryBarriers({region},
																							 0,
																							 VK_ACCESS_TRANSFER_WRITE_BIT,
																							 VK_IMAGE_LAYOUT_UNDEFINED,
																							 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));
					command_buffer->CmdCopy(staging_buffer, texture.m_image, {region});
					command_buffer->CmdPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
													   {}, {},
													   texture.m_image->GetDstMemoryBarriers({region},
																							 VK_ACCESS_TRANSFER_WRITE_BIT,
																							 VK_ACCESS_TRANSFER_READ_BIT,
																							 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
																							 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL));
					command_buffer->CmdGenerateMipmap2D(texture.m_image,
														VK_PIPELINE_STAGE_TRANSFER_BIT,
														VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
														VK_ACCESS_TRANSFER_READ_BIT,
														VK_ACCESS_SHADER_READ_BIT,
														VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
														VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

					command_buffer->End();
					tracker.Track(fence, {command_buffer, staging_buffer});
					tracker.Update();

					{
						std::lock_guard<std::mutex> lock_guard{mutex};
						command_buffer->Submit({}, {}, fence);
					}

					LOGI.printf("Texture %s loaded", texture_filenames[i].c_str());
				}
			}
		));
	}
}

void Scene::process_texture_errors() {
	if (m_textures.empty()) return;
	std::vector<uint32_t> new_index_mapper(m_textures.size()), prefix(m_textures.size(), 1u);
	//generate prefix
	for (uint32_t i = 0; i < m_textures.size(); ++i)
		prefix[i] = (m_textures[i].m_image != nullptr);
	//sum the prefix
	for (uint32_t i = 0; i < m_textures.size(); ++i) {
		if (prefix[i] == 0) new_index_mapper[i] = UINT32_MAX;
		if (i > 0) prefix[i] += prefix[i - 1];
		if (new_index_mapper[i] != UINT32_MAX) new_index_mapper[i] = prefix[i] - 1;
	}
	//move textures
	for (uint32_t i = 0; i < m_textures.size(); ++i) {
		if (new_index_mapper[i] != UINT32_MAX)
			m_textures[new_index_mapper[i]] = m_textures[i];
	}
	m_textures.resize(prefix.back());
	LOGI.printf("%u textures loaded", m_textures.size());
	//reindex draw commands
	for (auto &cmd : m_draw_commands) {
		if (cmd.m_push_constant.m_texture_id != UINT32_MAX) {
			cmd.m_push_constant.m_texture_id = new_index_mapper[cmd.m_push_constant.m_texture_id];
		}
	}
}

void Scene::create_descriptors(const std::shared_ptr<myvk::Device> &device) {
	{
		VkDescriptorSetLayoutBinding layout_binding = {};
		layout_binding.binding = 0;
		layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layout_binding.descriptorCount = std::max((uint32_t) m_textures.size(), 1u);
		layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(device, {layout_binding});
	}
	{
		VkDescriptorPoolSize pool_sizes[] = {
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, std::max((uint32_t) m_textures.size(), 1u)}
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1;
		pool_info.poolSizeCount = 1;
		pool_info.pPoolSizes = pool_sizes;

		m_descriptor_pool = myvk::DescriptorPool::Create(device, pool_info);
	}
	m_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_descriptor_set_layout);

	if (!m_textures.empty()) {
		std::vector<VkDescriptorImageInfo> image_infos(m_textures.size());
		for (uint32_t i = 0; i < m_textures.size(); ++i) {
			image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_infos[i].imageView = m_textures[i].m_image_view->GetHandle();
			image_infos[i].sampler = m_textures[i].m_sampler->GetHandle();
		}

		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_descriptor_set->GetHandle();
		write.dstBinding = 0;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = image_infos.size();
		write.pImageInfo = image_infos.data();

		vkUpdateDescriptorSets(device->GetHandle(), 1, &write, 0, nullptr);
	}
}

bool Scene::Initialize(const std::shared_ptr<myvk::Queue> &graphics_queue, const char *filename) {
	std::string base_dir = get_base_dir(filename);

	std::vector<Mesh> meshes;
	std::vector<std::string> texture_filenames;

	if (!load_meshes(filename, base_dir.c_str(), &meshes, &texture_filenames)) {
		LOGE.printf("Failed to load meshes");
		return false;
	}
	LOGI.printf("Meshes loaded from \"%s\"", filename);

	load_buffers_and_draw_cmd(graphics_queue, meshes);
	load_textures(graphics_queue, texture_filenames);
	process_texture_errors();
	create_descriptors(graphics_queue->GetDevicePtr());

	return true;
}

VkVertexInputBindingDescription Scene::GetVertexBindingDescription() {
	VkVertexInputBindingDescription ret = {};
	ret.binding = 0;
	ret.stride = sizeof(Vertex);
	ret.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return ret;
}

std::array<VkVertexInputAttributeDescription, 3> Scene::GetVertexAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 3> ret = {};
	ret[0].binding = 0;
	ret[0].location = 0;
	ret[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	ret[0].offset = offsetof(Vertex, m_position);

	ret[1].binding = 0;
	ret[1].location = 1;
	ret[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	ret[1].offset = offsetof(Vertex, m_normal);

	ret[2].binding = 0;
	ret[2].location = 2;
	ret[2].format = VK_FORMAT_R32G32_SFLOAT;
	ret[2].offset = offsetof(Vertex, m_texcoord);

	return ret;
}

VkPushConstantRange Scene::GetDefaultPushConstantRange() {
	VkPushConstantRange ret = {};
	ret.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	ret.offset = 0;
	ret.size = sizeof(uint32_t) * 2;
	return ret;
}

void Scene::CmdDraw(const std::shared_ptr<myvk::CommandBuffer> &command_buffer,
					const std::shared_ptr<myvk::PipelineLayout> &pipeline_layout,
					uint32_t push_constants_offset) const {
	command_buffer->CmdBindVertexBuffer(m_vertex_buffer, 0);
	command_buffer->CmdBindIndexBuffer(m_index_buffer, 0, VK_INDEX_TYPE_UINT32);
	for (const DrawCmd &draw_cmd : m_draw_commands) {
		command_buffer->CmdPushConstants(pipeline_layout,
										 VK_SHADER_STAGE_FRAGMENT_BIT,
										 push_constants_offset,
										 sizeof(uint32_t),
										 &draw_cmd.m_push_constant.m_texture_id);
		command_buffer->CmdPushConstants(pipeline_layout,
										 VK_SHADER_STAGE_FRAGMENT_BIT,
										 push_constants_offset + sizeof(uint32_t),
										 sizeof(uint32_t),
										 &draw_cmd.m_push_constant.m_albedo);
		command_buffer->CmdDrawIndexed(draw_cmd.m_index_count, 1, draw_cmd.m_first_index, 0, 0);
	}
}

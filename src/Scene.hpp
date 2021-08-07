#ifndef SCENE_HPP
#define SCENE_HPP

#include "myvk/Buffer.hpp"
#include "myvk/CommandBuffer.hpp"
#include "myvk/DescriptorPool.hpp"
#include "myvk/DescriptorSet.hpp"
#include "myvk/DescriptorSetLayout.hpp"
#include "myvk/Image.hpp"
#include "myvk/ImageView.hpp"
#include "myvk/Sampler.hpp"

#include <atomic>
#include <glm/glm.hpp>
#include <string>

class Scene {
private:
	struct Texture {
		std::shared_ptr<myvk::Image> m_image;
		std::shared_ptr<myvk::ImageView> m_image_view;
		std::shared_ptr<myvk::Sampler> m_sampler;
	};
	std::vector<Texture> m_textures;

	std::shared_ptr<myvk::Buffer> m_vertex_buffer, m_index_buffer;

	std::shared_ptr<myvk::DescriptorPool> m_descriptor_pool;
	std::shared_ptr<myvk::DescriptorSetLayout> m_descriptor_set_layout;
	std::shared_ptr<myvk::DescriptorSet> m_descriptor_set;

	struct PushConstant {
		uint32_t m_texture_id, m_albedo;
	};
	struct DrawCmd {
		PushConstant m_push_constant;
		uint32_t m_index_count, m_first_index;
	};
	struct Mesh;
	std::vector<DrawCmd> m_draw_commands;

	static bool load_meshes(const char *filename, const char *base_dir, std::vector<Mesh> *meshes,
	                        std::vector<std::string> *texture_filenames);

	void load_buffers_and_draw_cmd(const std::shared_ptr<myvk::Queue> &graphics_queue, const std::vector<Mesh> &meshes);

	void load_textures(const std::shared_ptr<myvk::Queue> &graphics_queue,
	                   const std::vector<std::string> &texture_filenames);

	void process_texture_errors();

	void create_descriptors(const std::shared_ptr<myvk::Device> &device);

public:
	static std::shared_ptr<Scene> Create(const std::shared_ptr<myvk::Queue> &graphics_queue, const char *filename,
	                                     std::atomic<const char *> *notification_ptr = nullptr);

	static std::vector<VkVertexInputBindingDescription> GetVertexBindingDescriptions();

	static std::vector<VkVertexInputAttributeDescription> GetVertexAttributeDescriptions();

	static VkPushConstantRange GetDefaultPushConstantRange();

	uint32_t GetTextureCount() const { return m_textures.size(); }

	const std::shared_ptr<myvk::DescriptorSet> &GetDescriptorSet() const { return m_descriptor_set; }

	const std::shared_ptr<myvk::DescriptorSetLayout> &GetDescriptorSetLayout() const { return m_descriptor_set_layout; }

	void CmdDraw(const std::shared_ptr<myvk::CommandBuffer> &command_buffer,
	             const std::shared_ptr<myvk::PipelineLayout> &pipeline_layout,
	             uint32_t push_constants_offset = 0) const;
};

#endif

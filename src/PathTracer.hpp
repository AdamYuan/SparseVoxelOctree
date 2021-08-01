#ifndef PATH_TRACER_HPP
#define PATH_TRACER_HPP

#include "Camera.hpp"
#include "Config.hpp"
#include "Lighting.hpp"
#include "Octree.hpp"
#include "Sobol.hpp"
#include "myvk/Image.hpp"

#include <glm/glm.hpp>

class PathTracer {
private:
	std::shared_ptr<Octree> m_octree_ptr;
	std::shared_ptr<Camera> m_camera_ptr;
	std::shared_ptr<Lighting> m_lighting_ptr;

	Sobol m_sobol;
	std::shared_ptr<myvk::Image> m_color_image, m_albedo_image, m_normal_image, m_noise_image;
	std::shared_ptr<myvk::ImageView> m_color_image_view, m_albedo_image_view, m_normal_image_view, m_noise_image_view;
	std::shared_ptr<myvk::Sampler> m_noise_sampler;

	std::shared_ptr<myvk::DescriptorPool> m_descriptor_pool;
	std::shared_ptr<myvk::DescriptorSetLayout> m_target_descriptor_set_layout, m_noise_descriptor_set_layout;
	std::shared_ptr<myvk::DescriptorSet> m_target_descriptor_set, m_noise_descriptor_set;

	std::shared_ptr<myvk::PipelineLayout> m_pipeline_layout;
	std::shared_ptr<myvk::ComputePipeline> m_pipeline;

	void create_target_images(const std::shared_ptr<myvk::Device> &device,
	                          const std::vector<std::shared_ptr<myvk::Queue>> &access_queue);
	void create_noise_images(const std::shared_ptr<myvk::Device> &device);
	void create_descriptor(const std::shared_ptr<myvk::Device> &device);
	void create_pipeline(const std::shared_ptr<myvk::Device> &device);

	void clear_target_images(const std::shared_ptr<myvk::CommandPool> &command_pool);
	void set_noise_image(const std::shared_ptr<myvk::CommandPool> &command_pool);

	static void extract_target_image_to_buffer(const std::shared_ptr<myvk::CommandPool> &command_pool,
	                                           const std::shared_ptr<myvk::ImageBase> &image,
	                                           const std::shared_ptr<myvk::BufferBase> &buffer);

public:
	uint32_t m_width{kDefaultWidth}, m_height{kDefaultHeight};
	uint32_t m_bounce;

	static std::shared_ptr<PathTracer> Create(const std::shared_ptr<Octree> &octree,
	                                          const std::shared_ptr<Camera> &camera,
	                                          const std::shared_ptr<Lighting> &lighting,
	                                          const std::shared_ptr<myvk::CommandPool> &command_pool);
	const std::shared_ptr<Octree> &GetOctreePtr() const { return m_octree_ptr; }
	const std::shared_ptr<Camera> &GetCameraPtr() const { return m_camera_ptr; }
	const std::shared_ptr<Lighting> &GetLightingPtr() const { return m_lighting_ptr; }

	void Reset(const std::shared_ptr<myvk::CommandPool> &command_pool,
	           const std::shared_ptr<myvk::Queue> &shared_queue);

	void CmdRender(const std::shared_ptr<myvk::CommandBuffer> &command_buffer);

	const std::shared_ptr<myvk::Image> &GetColorImage() const { return m_color_image; }
	const std::shared_ptr<myvk::Image> &GetAlbedoImage() const { return m_albedo_image; }
	const std::shared_ptr<myvk::Image> &GetNormalImage() const { return m_normal_image; }

	const std::shared_ptr<myvk::DescriptorSetLayout> &GetTargetDescriptorSetLayout() const {
		return m_target_descriptor_set_layout;
	}
	const std::shared_ptr<myvk::DescriptorSet> &GetTargetDescriptorSet() const { return m_target_descriptor_set; }

	std::vector<float> ExtractColorImage(const std::shared_ptr<myvk::CommandPool> &command_pool) const;
	std::vector<float> ExtractAlbedoImage(const std::shared_ptr<myvk::CommandPool> &command_pool) const;
	std::vector<float> ExtractNormalImage(const std::shared_ptr<myvk::CommandPool> &command_pool) const;
};

#endif

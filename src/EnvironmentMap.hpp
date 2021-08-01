#ifndef ENVIRONMENT_MAP_HPP
#define ENVIRONMENT_MAP_HPP

#include <myvk/Buffer.hpp>
#include <myvk/CommandPool.hpp>
#include <myvk/DescriptorSet.hpp>
#include <myvk/Image.hpp>
#include <myvk/Sampler.hpp>

class EnvironmentMap {
private:
	struct HdrImg {
		uint32_t m_width, m_height;
		float *m_data;
	};
	std::shared_ptr<myvk::Sampler> m_sampler;
	std::shared_ptr<myvk::Image> m_image;
	std::shared_ptr<myvk::ImageView> m_image_view;
	std::shared_ptr<myvk::Buffer> m_prab_buffer, m_alias_buffer;

	std::shared_ptr<myvk::DescriptorPool> m_descriptor_pool;
	std::shared_ptr<myvk::DescriptorSetLayout> m_descriptor_set_layout;
	std::shared_ptr<myvk::DescriptorSet> m_descriptor_set;

	static HdrImg load_hdr_image(const char *filename);
	static void weigh_hdr_image(const HdrImg *img);

	void create_images_and_buffers(const std::shared_ptr<myvk::CommandPool> &command_pool, const HdrImg &img,
	                               const std::vector<float> &prab_vector, const std::vector<uint32_t> &alias_vector);
	void create_descriptors(const std::shared_ptr<myvk::Device> &device);

public:
	static std::shared_ptr<EnvironmentMap> Create(const std::shared_ptr<myvk::Device> &device);
	const std::shared_ptr<myvk::DescriptorSetLayout> &GetDescriptorSetLayout() const { return m_descriptor_set_layout; }
	const std::shared_ptr<myvk::DescriptorSet> &GetDescriptorSet() const { return m_descriptor_set; }
	const VkExtent3D &GetImageExtent() const { return m_image->GetExtent(); }
	bool Empty() const { return m_image == nullptr; }
	void Reset();
	void Reset(const std::shared_ptr<myvk::CommandPool> &command_pool, const char *filename);
};

#endif

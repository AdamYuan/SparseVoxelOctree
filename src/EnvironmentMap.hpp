#ifndef ENVIRONMENT_MAP_HPP
#define ENVIRONMENT_MAP_HPP

#include <myvk/Buffer.hpp>
#include <myvk/CommandBuffer.hpp>
#include <myvk/CommandPool.hpp>
#include <myvk/DescriptorSet.hpp>
#include <myvk/Image.hpp>
#include <myvk/Sampler.hpp>

class EnvironmentMap {
public:
	float m_rotation{0.0f}, m_multiplier{1.0f};

private:
	struct HdrImg {
		uint32_t m_width, m_height;
		float *m_data;
	};
	struct AliasPair {
		uint32_t m_prob, m_alias; // m_prob is normalized to [0, 2^32 - 1]
	};
	std::shared_ptr<myvk::Sampler> m_sampler;
	std::shared_ptr<myvk::Image> m_hdr_image, m_alias_table_image;
	std::shared_ptr<myvk::ImageView> m_hdr_image_view, m_alias_table_image_view;

	std::shared_ptr<myvk::DescriptorPool> m_descriptor_pool;
	std::shared_ptr<myvk::DescriptorSetLayout> m_descriptor_set_layout;
	std::shared_ptr<myvk::DescriptorSet> m_descriptor_set;

	static HdrImg load_hdr_image(const char *filename);
	static std::vector<double> weigh_hdr_image(const HdrImg *img);
	static void generate_alias_table(std::vector<double> *weights_ptr, AliasPair *alias_table);

	void create_images(const std::shared_ptr<myvk::CommandPool> &command_pool, const HdrImg &img,
	                   std::vector<double> *weights_ptr);
	void create_descriptors(const std::shared_ptr<myvk::Device> &device);

public:
	static std::shared_ptr<EnvironmentMap> Create(const std::shared_ptr<myvk::Device> &device);
	const std::shared_ptr<myvk::DescriptorSetLayout> &GetDescriptorSetLayout() const { return m_descriptor_set_layout; }
	const std::shared_ptr<myvk::DescriptorSet> &GetDescriptorSet() const { return m_descriptor_set; }
	const VkExtent3D &GetImageExtent() const { return m_hdr_image->GetExtent(); }
	bool Empty() const { return m_hdr_image == nullptr; }
	void Reset();
	void Reset(const std::shared_ptr<myvk::CommandPool> &command_pool, const char *filename);

	void CmdTransferOwnership(const std::shared_ptr<myvk::CommandBuffer> &command_buffer, uint32_t src_queue_family,
	                          uint32_t dst_queue_family,
	                          VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	                          VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) const;
};

#endif

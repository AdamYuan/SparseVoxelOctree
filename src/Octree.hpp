#ifndef OCTREE_HPP
#define OCTREE_HPP

#include "OctreeBuilder.hpp"
#include "myvk/Buffer.hpp"
#include "myvk/DescriptorSet.hpp"

class Octree {
private:
	std::shared_ptr<myvk::Buffer> m_buffer;
	VkDeviceSize m_range{};
	uint32_t m_level{};
	std::shared_ptr<myvk::DescriptorPool> m_descriptor_pool;
	std::shared_ptr<myvk::DescriptorSetLayout> m_descriptor_set_layout;
	std::shared_ptr<myvk::DescriptorSet> m_descriptor_set;

public:
	static std::shared_ptr<Octree> Create(const std::shared_ptr<myvk::Device> &device);

	void Update(const std::shared_ptr<myvk::CommandPool> &command_pool, const std::shared_ptr<OctreeBuilder> &builder);
	bool Empty() const { return m_buffer == nullptr; }

	const std::shared_ptr<myvk::Buffer> &GetBuffer() const { return m_buffer; }
	const std::shared_ptr<myvk::DescriptorSetLayout> &GetDescriptorSetLayout() const { return m_descriptor_set_layout; }
	const std::shared_ptr<myvk::DescriptorSet> &GetDescriptorSet() const { return m_descriptor_set; }

	uint32_t GetLevel() const { return m_level; }
	VkDeviceSize GetRange() const { return m_range; }

	void CmdTransferOwnership(const std::shared_ptr<myvk::CommandBuffer> &command_buffer, uint32_t src_queue_family,
	                          uint32_t dst_queue_family,
	                          VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	                          VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) const;
};

#endif

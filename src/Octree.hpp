#ifndef OCTREE_HPP
#define OCTREE_HPP

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
	void Initialize(const std::shared_ptr<myvk::Device> &device);

	void Update(const std::shared_ptr<myvk::Buffer> &buffer, uint32_t level, VkDeviceSize range);

	bool Empty() const { return m_buffer == nullptr; }

	const std::shared_ptr<myvk::Buffer> &GetBuffer() const { return m_buffer; }

	const std::shared_ptr<myvk::DescriptorSetLayout> &GetDescriptorSetLayout() const {
		return m_descriptor_set_layout;
	}

	const std::shared_ptr<myvk::DescriptorSet> &GetDescriptorSet() const { return m_descriptor_set; }

	uint32_t GetLevel() const { return m_level; }

	VkDeviceSize GetRange() const { return m_range; }
};

#endif

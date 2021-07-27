#ifndef ENVIRONMENT_MAP_HPP
#define ENVIRONMENT_MAP_HPP

#include <myvk/Buffer.hpp>
#include <myvk/CommandPool.hpp>
#include <myvk/DescriptorSet.hpp>
#include <myvk/Image.hpp>
#include <myvk/Sampler.hpp>

class EnvironmentMap {
private:
	std::shared_ptr<myvk::Image> m_image;
	std::shared_ptr<myvk::Buffer> m_prab_buffer, m_alias_buffer;

	std::shared_ptr<myvk::DescriptorPool> m_descriptor_pool;
	std::shared_ptr<myvk::DescriptorSetLayout> m_descriptor_set_layout;
	std::shared_ptr<myvk::DescriptorSet> m_descriptor_set;

	void create_descriptors(const std::shared_ptr<myvk::Device> &device);

public:
	static std::shared_ptr<EnvironmentMap> Create(const std::shared_ptr<myvk::Device> &device);
	bool Empty() const { return m_image == nullptr; }
	void Reset();
	void Reset(const std::shared_ptr<myvk::CommandPool> &m_command_pool, const char *filename);
};

#endif

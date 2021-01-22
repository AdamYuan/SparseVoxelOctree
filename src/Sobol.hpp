#ifndef SOBOL_HPP
#define SOBOL_HPP

#include "myvk/Buffer.hpp"
#include "myvk/CommandBuffer.hpp"
#include "myvk/ComputePipeline.hpp"
#include <vector>

class Sobol {
private:
	std::shared_ptr<myvk::Buffer> m_sobol_buffer, m_staging_buffer;
	std::shared_ptr<myvk::DescriptorPool> m_descriptor_pool;
	std::shared_ptr<myvk::DescriptorSetLayout> m_descriptor_set_layout;
	std::shared_ptr<myvk::DescriptorSet> m_descriptor_set;
	std::shared_ptr<myvk::PipelineLayout> m_pipeline_layout;
	std::shared_ptr<myvk::ComputePipeline> m_compute_pipeline;
	uint32_t m_dimension{};

public:
	void Initialize(const std::shared_ptr<myvk::Device> &device);

	void CmdNext(const std::shared_ptr<myvk::CommandBuffer> &command_buffer);

	const std::shared_ptr<myvk::DescriptorSetLayout> &GetDescriptorSetLayout() const { return m_descriptor_set_layout; }

	const std::shared_ptr<myvk::DescriptorSet> &GetDescriptorSet() const { return m_descriptor_set; }

	const std::shared_ptr<myvk::Buffer> &GetBuffer() const { return m_sobol_buffer; }

	void Reset(const std::shared_ptr<myvk::CommandPool> &command_pool, uint32_t dimension);

	uint32_t GetDimension() const { return m_dimension; }
};

#endif

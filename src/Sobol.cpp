#include "Sobol.hpp"

constexpr uint32_t kMaxDimension = 64;
constexpr VkDeviceSize kBufferSize = (kMaxDimension + 1) * sizeof(uint32_t);

/*void Sobol::Next(float *out) {
    uint8_t c = get_first_zero_bit(m_index++);
    //uint8_t c = glm::findLSB(~(m_index ++));
    for (unsigned j = 0; j < m_dim; ++j)
       out[j] = (float) ((m_x[j] ^= kMatrices[j][c]) / 4294967296.0);
}*/

void Sobol::Initialize(const std::shared_ptr<myvk::Device> &device) {
	m_descriptor_pool = myvk::DescriptorPool::Create(device, 1, {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}});
	{
		VkDescriptorSetLayoutBinding sobol_binding = {};
		sobol_binding.binding = 0;
		sobol_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		sobol_binding.descriptorCount = 1;
		sobol_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(device, {sobol_binding});
	}
	m_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_descriptor_set_layout);

	m_sobol_buffer = myvk::Buffer::Create(device, kBufferSize, VMA_MEMORY_USAGE_GPU_ONLY,
	                                      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	m_descriptor_set->UpdateStorageBuffer(m_sobol_buffer, 0);

	m_staging_buffer = myvk::Buffer::CreateStaging(device, kBufferSize);
	{
		uint32_t *data = (uint32_t *)m_staging_buffer->Map();
		std::fill(data, data + kMaxDimension + 1, 0u);
		m_staging_buffer->Unmap();
	}

	m_pipeline_layout = myvk::PipelineLayout::Create(device, {m_descriptor_set_layout},
	                                                 {{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)}});
	{
		constexpr uint32_t kSobolCompSpv[] = {
#include "spirv/sobol.comp.u32"
		};
		std::shared_ptr<myvk::ShaderModule> sobol_shader_module =
		    myvk::ShaderModule::Create(device, kSobolCompSpv, sizeof(kSobolCompSpv));
		m_compute_pipeline = myvk::ComputePipeline::Create(m_pipeline_layout, sobol_shader_module);
	}
}

void Sobol::CmdNext(const std::shared_ptr<myvk::CommandBuffer> &command_buffer) {
	command_buffer->CmdBindDescriptorSets({m_descriptor_set}, m_pipeline_layout, VK_PIPELINE_BIND_POINT_COMPUTE, {});
	command_buffer->CmdPushConstants(m_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &m_dimension);
	command_buffer->CmdBindPipeline(m_compute_pipeline);
	command_buffer->CmdDispatch(1, 1, 1);
}

void Sobol::Reset(const std::shared_ptr<myvk::CommandPool> &command_pool, uint32_t dimension) {
	m_dimension = dimension;

	std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(command_pool->GetDevicePtr());
	std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(command_pool);
	command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	command_buffer->CmdCopy(m_staging_buffer, m_sobol_buffer, {{0, 0, kBufferSize}});
	command_buffer->End();
	command_buffer->Submit(fence);
	fence->Wait();
}

#include "Sobol.hpp"
#include "myvk/Buffer.hpp"
#include "vulkan/vulkan_core.h"
constexpr uint32_t kMaxDimension = 64;

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
	m_sobol_buffer = myvk::Buffer::Create(device, (kMaxDimension + 1) * sizeof(uint32_t), VMA_MEMORY_USAGE_GPU_ONLY,
	                                      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	m_descriptor_set->UpdateStorageBuffer(m_sobol_buffer, 0);
}

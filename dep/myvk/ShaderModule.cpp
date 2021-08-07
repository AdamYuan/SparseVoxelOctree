#include "ShaderModule.hpp"

namespace myvk {

std::shared_ptr<ShaderModule> ShaderModule::Create(const std::shared_ptr<Device> &device, const uint32_t *code,
                                                   uint32_t code_size) {
	std::shared_ptr<ShaderModule> ret = std::make_shared<ShaderModule>();
	ret->m_device_ptr = device;

	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.pCode = code;
	info.codeSize = code_size;
	if (vkCreateShaderModule(device->GetHandle(), &info, nullptr, &ret->m_shader_module) != VK_SUCCESS)
		return nullptr;
	return ret;
}

ShaderModule::~ShaderModule() {
	if (m_shader_module)
		vkDestroyShaderModule(m_device_ptr->GetHandle(), m_shader_module, nullptr);
}

VkPipelineShaderStageCreateInfo
ShaderModule::GetPipelineShaderStageCreateInfo(VkShaderStageFlagBits stage,
                                               VkSpecializationInfo *p_specialization_info) const {
	VkPipelineShaderStageCreateInfo ret = {};
	ret.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	ret.stage = stage;
	ret.module = m_shader_module;
	ret.pName = "main";
	ret.pSpecializationInfo = p_specialization_info;
	return ret;
}
} // namespace myvk

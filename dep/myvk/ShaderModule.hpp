#ifndef MYVK_SHADER_MODULE_HPP
#define MYVK_SHADER_MODULE_HPP

#include "DeviceObjectBase.hpp"
#include <memory>
#include <volk.h>

namespace myvk {
class ShaderModule : public DeviceObjectBase {
private:
	std::shared_ptr<Device> m_device_ptr;
	VkShaderModule m_shader_module{VK_NULL_HANDLE};
	VkShaderStageFlagBits m_stage;

public:
	static std::shared_ptr<ShaderModule> Create(const std::shared_ptr<Device> &device, const uint32_t *code,
	                                            uint32_t code_size);

	VkShaderStageFlagBits GetStage() const { return m_stage; }
	VkShaderModule GetHandle() const { return m_shader_module; }

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	VkPipelineShaderStageCreateInfo
	GetPipelineShaderStageCreateInfo(VkShaderStageFlagBits stage,
	                                 VkSpecializationInfo *p_specialization_info = nullptr) const;

	~ShaderModule();
};
} // namespace myvk

#endif

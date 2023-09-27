#ifndef MYVK_SHADER_MODULE_HPP
#define MYVK_SHADER_MODULE_HPP

#include "DeviceObjectBase.hpp"
#include "volk.h"
#include <memory>

namespace myvk {
class ShaderModule : public DeviceObjectBase {
private:
	Ptr<Device> m_device_ptr;
	VkShaderModule m_shader_module{VK_NULL_HANDLE};
	VkShaderStageFlagBits m_stage;

public:
	static Ptr<ShaderModule> Create(const Ptr<Device> &device, const uint32_t *code, uint32_t code_size);

	VkShaderStageFlagBits GetStage() const { return m_stage; }
	VkShaderModule GetHandle() const { return m_shader_module; }

	const Ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	VkPipelineShaderStageCreateInfo
	GetPipelineShaderStageCreateInfo(VkShaderStageFlagBits stage,
	                                 VkSpecializationInfo *p_specialization_info = nullptr) const;

	~ShaderModule() override;
};
} // namespace myvk

#endif

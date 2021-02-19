#ifndef MYVK_SAMPLER_HPP
#define MYVK_SAMPLER_HPP

#include "DeviceObjectBase.hpp"

namespace myvk {
class Sampler : public DeviceObjectBase {
private:
	std::shared_ptr<Device> m_device_ptr;

	VkSampler m_sampler{VK_NULL_HANDLE};

public:
	static std::shared_ptr<Sampler> Create(const std::shared_ptr<Device> &device,
	                                       const VkSamplerCreateInfo &create_info);

	static std::shared_ptr<Sampler> Create(const std::shared_ptr<Device> &device, VkFilter filter,
	                                       VkSamplerAddressMode address_mode,
	                                       VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
	                                       uint32_t mipmap_level = 1, bool request_anisotropy = false,
	                                       float max_anisotropy = 1.0f);

	static std::shared_ptr<Sampler> CreateClampToBorder(const std::shared_ptr<Device> &device, VkFilter filter,
	                                                    VkBorderColor border_color,
	                                                    VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
	                                                    uint32_t mipmap_level = 1, bool request_anisotropy = false,
	                                                    float max_anisotropy = 1.0f);

	VkSampler GetHandle() const { return m_sampler; }

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_device_ptr; };

	~Sampler();
};
} // namespace myvk

#endif

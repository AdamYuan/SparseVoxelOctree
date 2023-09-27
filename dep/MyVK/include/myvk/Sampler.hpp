#ifndef MYVK_SAMPLER_HPP
#define MYVK_SAMPLER_HPP

#include "DeviceObjectBase.hpp"

namespace myvk {
class Sampler : public DeviceObjectBase {
private:
	Ptr<Device> m_device_ptr;

	VkSampler m_sampler{VK_NULL_HANDLE};

public:
	static Ptr<Sampler> Create(const Ptr<Device> &device, const VkSamplerCreateInfo &create_info);

	static Ptr<Sampler> Create(const Ptr<Device> &device, VkFilter filter, VkSamplerAddressMode address_mode,
	                           VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
	                           float max_lod = VK_LOD_CLAMP_NONE, bool request_anisotropy = false, float max_anisotropy = 1.0f);

	static Ptr<Sampler> CreateClampToBorder(const Ptr<Device> &device, VkFilter filter, VkBorderColor border_color,
	                                        VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
	                                        float max_lod = VK_LOD_CLAMP_NONE, bool request_anisotropy = false,
	                                        float max_anisotropy = 1.0f);

	VkSampler GetHandle() const { return m_sampler; }

	const Ptr<Device> &GetDevicePtr() const override { return m_device_ptr; };

	~Sampler() override;
};
} // namespace myvk

#endif

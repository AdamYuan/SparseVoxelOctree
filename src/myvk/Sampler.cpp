#include "Sampler.hpp"

namespace myvk {
std::shared_ptr<Sampler> Sampler::Create(const std::shared_ptr<Device> &device,
                                         const VkSamplerCreateInfo &create_info) {
	std::shared_ptr<Sampler> ret = std::make_shared<Sampler>();
	ret->m_device_ptr = device;

	if (vkCreateSampler(device->GetHandle(), &create_info, nullptr, &ret->m_sampler) != VK_SUCCESS)
		return nullptr;
	return ret;
}

std::shared_ptr<Sampler> Sampler::Create(const std::shared_ptr<Device> &device, VkFilter filter,
                                         VkSamplerAddressMode address_mode, VkSamplerMipmapMode mipmap_mode,
                                         uint32_t mipmap_level, bool request_anisotropy, float max_anisotropy) {
	VkSamplerCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	create_info.magFilter = filter;
	create_info.minFilter = filter;
	create_info.mipmapMode = mipmap_mode;
	create_info.addressModeU = address_mode;
	create_info.addressModeV = address_mode;
	create_info.addressModeW = address_mode;
	create_info.mipLodBias = 0.0f;
	create_info.compareOp = VK_COMPARE_OP_NEVER;
	create_info.minLod = 0.0f;
	create_info.maxLod = (float)mipmap_level;
	if (device->GetPhysicalDevicePtr()->GetFeatures().samplerAnisotropy) {
		create_info.anisotropyEnable = request_anisotropy ? VK_TRUE : VK_FALSE;
		create_info.maxAnisotropy = max_anisotropy;
	} else {
		create_info.anisotropyEnable = VK_FALSE;
		create_info.maxAnisotropy = 1.0;
	}
	create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	return Create(device, create_info);
}

Sampler::~Sampler() {
	if (m_sampler)
		vkDestroySampler(m_device_ptr->GetHandle(), m_sampler, nullptr);
}

std::shared_ptr<Sampler> Sampler::CreateClampToBorder(const std::shared_ptr<Device> &device, VkFilter filter,
                                                      VkBorderColor border_color, VkSamplerMipmapMode mipmap_mode,
                                                      uint32_t mipmap_level, bool request_anisotropy,
                                                      float max_anisotropy) {
	VkSamplerCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	create_info.magFilter = filter;
	create_info.minFilter = filter;
	create_info.mipmapMode = mipmap_mode;
	create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	create_info.borderColor = border_color;
	create_info.mipLodBias = 0.0f;
	create_info.compareOp = VK_COMPARE_OP_NEVER;
	create_info.minLod = 0.0f;
	create_info.maxLod = (float)mipmap_level;
	if (device->GetPhysicalDevicePtr()->GetFeatures().samplerAnisotropy) {
		create_info.anisotropyEnable = request_anisotropy ? VK_TRUE : VK_FALSE;
		create_info.maxAnisotropy = max_anisotropy;
	} else {
		create_info.maxAnisotropy = 1.0;
		create_info.anisotropyEnable = VK_FALSE;
	}

	return Create(device, create_info);
}
} // namespace myvk

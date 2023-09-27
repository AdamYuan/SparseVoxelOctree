#ifndef MYVK_RG_VK_HELPER_HPP
#define MYVK_RG_VK_HELPER_HPP

#include <volk.h>

inline static void UpdateVkImageTypeFromVkImageViewType(VkImageType *p_image_type, VkImageViewType view_type) {
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageViewCreateInfo.html
	switch (view_type) {
	case VK_IMAGE_VIEW_TYPE_1D:
	case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
		*p_image_type = VK_IMAGE_TYPE_1D;
		return;
	case VK_IMAGE_VIEW_TYPE_CUBE:
	case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
		*p_image_type = VK_IMAGE_TYPE_2D;
		return;
	case VK_IMAGE_VIEW_TYPE_2D:
	case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
		if (*p_image_type == VK_IMAGE_TYPE_1D)
			*p_image_type = VK_IMAGE_TYPE_2D;
		return;
	case VK_IMAGE_VIEW_TYPE_3D:
		*p_image_type = VK_IMAGE_TYPE_3D;
		return;
	default:
		return;
	}
}

inline static constexpr VkImageAspectFlags VkImageAspectFlagsFromVkFormat(VkFormat format) {
	switch (format) {
	case VK_FORMAT_D32_SFLOAT:
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_X8_D24_UNORM_PACK32:
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	case VK_FORMAT_D16_UNORM_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	case VK_FORMAT_S8_UINT:
		return VK_IMAGE_ASPECT_STENCIL_BIT;
	default:
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

inline static constexpr VkPipelineStageFlags2 VkAttachmentInitialStagesFromVkFormat(VkFormat format) {
	VkImageAspectFlags aspects = VkImageAspectFlagsFromVkFormat(format);
	VkPipelineStageFlags2 ret{};
	if (aspects & VK_IMAGE_ASPECT_COLOR_BIT)
		ret |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	if (aspects & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
		ret |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
	return ret;
}

inline static constexpr VkAccessFlags2 VkAttachmentInitAccessFromVkFormat(VkFormat format) {
	VkImageAspectFlags aspects = VkImageAspectFlagsFromVkFormat(format);
	VkAccessFlags2 ret{};
	if (aspects & VK_IMAGE_ASPECT_COLOR_BIT)
		ret |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	if (aspects & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
		ret |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	return ret;
}

inline static constexpr VkAccessFlags2 VkAttachmentLoadAccessFromVkFormat(VkFormat format) {
	VkImageAspectFlags aspects = VkImageAspectFlagsFromVkFormat(format);
	VkAccessFlags2 ret{};
	if (aspects & VK_IMAGE_ASPECT_COLOR_BIT)
		ret |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
	if (aspects & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
		ret |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	return ret;
}

inline static constexpr VkShaderStageFlags VkShaderStagesFromVkPipelineStages(VkPipelineStageFlags2 pipeline_stages) {
	VkShaderStageFlags ret = 0;
	if (pipeline_stages & VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT)
		ret |= VK_SHADER_STAGE_VERTEX_BIT;
	if (pipeline_stages & VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT)
		ret |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	if (pipeline_stages & VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT)
		ret |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	if (pipeline_stages & VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT)
		ret |= VK_SHADER_STAGE_GEOMETRY_BIT;
	if (pipeline_stages & VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT)
		ret |= VK_SHADER_STAGE_FRAGMENT_BIT;
	if (pipeline_stages & VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT)
		ret |= VK_SHADER_STAGE_COMPUTE_BIT;
	return ret;
}

#endif

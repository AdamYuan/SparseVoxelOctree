#include "EnvironmentMap.hpp"
#include <cmath>
#include <myvk/CommandBuffer.hpp>
#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <tinyexr.h>

std::shared_ptr<EnvironmentMap> EnvironmentMap::Create(const std::shared_ptr<myvk::Device> &device) {
	std::shared_ptr<EnvironmentMap> ret = std::make_shared<EnvironmentMap>();
	ret->m_sampler = myvk::Sampler::Create(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	ret->create_descriptors(device);
	return ret;
}

void EnvironmentMap::create_descriptors(const std::shared_ptr<myvk::Device> &device) {
	m_descriptor_pool = myvk::DescriptorPool::Create(
	    device, 1, {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}});
	{
		VkDescriptorSetLayoutBinding image_binding = {};
		image_binding.binding = 0;
		image_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		image_binding.descriptorCount = 1;
		image_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
		VkSampler immutable_samplers[] = {m_sampler->GetHandle()};
		image_binding.pImmutableSamplers = immutable_samplers;

		VkDescriptorSetLayoutBinding prab_buffer_binding = {};
		prab_buffer_binding.binding = 1;
		prab_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		prab_buffer_binding.descriptorCount = 1;
		prab_buffer_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding alias_buffer_binding = {};
		alias_buffer_binding.binding = 2;
		alias_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		alias_buffer_binding.descriptorCount = 1;
		alias_buffer_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(
		    device, {{image_binding, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
		             {prab_buffer_binding, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
		             {alias_buffer_binding, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT}});
	}
	m_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_descriptor_set_layout);
}

void EnvironmentMap::Reset() {
	m_image = nullptr;
	m_image_view = nullptr;
	m_prab_buffer = nullptr;
	m_alias_buffer = nullptr;
}

void EnvironmentMap::Reset(const std::shared_ptr<myvk::CommandPool> &command_pool, const char *filename) {
	HdrImg img = load_hdr_image(filename);
	if (!img.m_data) {
		Reset();
		return;
	}

	// TODO: calculate weights, copy to vulkan image
	weigh_hdr_image(&img);
	create_images_and_buffers(command_pool, img);

	free(img.m_data); // release hdr image data

	m_descriptor_set->UpdateCombinedImageSampler(m_sampler, m_image_view, 0);
}

EnvironmentMap::HdrImg EnvironmentMap::load_hdr_image(const char *filename) {
	constexpr uint64_t kMaxSize = UINT32_MAX / sizeof(float) / 4;
	int w, h;
	float *rgba = nullptr;
	if (IsEXR(filename) == TINYEXR_SUCCESS) {
		char *err = nullptr;
		int res = LoadEXR(&rgba, &w, &h, filename, (const char **)&err);
		if (err)
			spdlog::error(err);
		free(err);
		if (res != TINYEXR_SUCCESS)
			spdlog::error("failed to load {} environment map as EXR file", filename);
		else if ((uint64_t)w * h > kMaxSize)
			spdlog::error("environment map {} ({}x{}) is too large", filename, w, h);
		else {
			spdlog::info("environment map {} ({}x{}) loaded", filename, w, h);
			return {(uint32_t)w, (uint32_t)h, rgba};
		}
	} else if (stbi_is_hdr(filename)) {
		int channels;
		rgba = stbi_loadf(filename, &w, &h, &channels, 4);
		if (!rgba)
			spdlog::error("failed to load {} environment map as HDR file", filename);
		else if ((uint64_t)w * h > kMaxSize)
			spdlog::error("environment map {} ({}x{}) is too large", filename, w, h);
		else {
			spdlog::info("environment map {} ({}x{}) loaded", filename, w, h);
			return {(uint32_t)w, (uint32_t)h, rgba};
		}
	} else
		spdlog::error("{} is not supported as environment map, use *.exr or *.hdr instead", filename);

	free(rgba);
	return {0, 0, nullptr};
}

inline static float luminance(float r, float g, float b) { return r * 0.3f + g * 0.6f + b * 0.1f; }
void EnvironmentMap::weigh_hdr_image(const EnvironmentMap::HdrImg *img) {
	float *cur = img->m_data;
	double weight_sum = 0.0;
	for (uint32_t j = 0; j < img->m_height; ++j) {
		float sin_theta = sinf(M_PI * (1.0f - float(j) / float(img->m_height - 1)));
		for (uint32_t i = 0; i < img->m_width; ++i, cur += 4) {
			cur[3] = luminance(cur[0], cur[1], cur[2]) * sin_theta;
			weight_sum += (double)cur[3];
		}
	}
	spdlog::info("weight_sum = {}", weight_sum);
	double inv_weight_sum = 1.0 / weight_sum;
	// spdlog::info("inv_weight_sum = {}", inv_weight_sum);
	cur = img->m_data;
	for (uint32_t j = 0; j < img->m_height; ++j)
		for (uint32_t i = 0; i < img->m_width; ++i, cur += 4)
			cur[3] = float((double)cur[3] * inv_weight_sum);
}

void EnvironmentMap::create_images_and_buffers(const std::shared_ptr<myvk::CommandPool> &command_pool,
                                               const HdrImg &img) {
	const uint32_t img_size = img.m_width * img.m_height;
	const VkDeviceSize img_bytes = img_size * sizeof(float) * 4;

	const std::shared_ptr<myvk::Device> &device = command_pool->GetDevicePtr();

	// create vulkan objects
	m_image = myvk::Image::CreateTexture2D(device, {img.m_width, img.m_height}, 1, VK_FORMAT_R32G32B32A32_SFLOAT,
	                                       VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	m_image_view = myvk::ImageView::Create(m_image, VK_IMAGE_VIEW_TYPE_2D);

	// create and fill staging objects
	std::shared_ptr<myvk::Buffer> image_staging = myvk::Buffer::CreateStaging(device, img_bytes);
	image_staging->UpdateData(img.m_data, img.m_data + img_size * 4);

	// copy data
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {img.m_width, img.m_height, 1};

	std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(command_pool);
	std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(device);
	command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	command_buffer->CmdPipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {},
	                                   m_image->GetDstMemoryBarriers({region}, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
	                                                                 VK_IMAGE_LAYOUT_UNDEFINED,
	                                                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));
	command_buffer->CmdCopy(image_staging, m_image, {region});
	command_buffer->CmdPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {}, {},
	                                   m_image->GetDstMemoryBarriers({region}, VK_ACCESS_TRANSFER_WRITE_BIT, 0,
	                                                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                                                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
	command_buffer->End();

	command_buffer->Submit(fence);
	fence->Wait();
}

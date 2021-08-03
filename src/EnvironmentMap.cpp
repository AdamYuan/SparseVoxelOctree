#include "EnvironmentMap.hpp"
#include <glm/glm.hpp>
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
	m_descriptor_pool = myvk::DescriptorPool::Create(device, 1, {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2}});
	{
		VkSampler immutable_samplers[] = {m_sampler->GetHandle()};

		VkDescriptorSetLayoutBinding hdr_image_binding = {};
		hdr_image_binding.binding = 0;
		hdr_image_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		hdr_image_binding.descriptorCount = 1;
		hdr_image_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
		hdr_image_binding.pImmutableSamplers = immutable_samplers;

		VkDescriptorSetLayoutBinding alias_table_image_binding = {};
		alias_table_image_binding.binding = 1;
		alias_table_image_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		alias_table_image_binding.descriptorCount = 1;
		alias_table_image_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		alias_table_image_binding.pImmutableSamplers = immutable_samplers;

		m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(
		    device, {{hdr_image_binding, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
		             {alias_table_image_binding, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT}});
	}
	m_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_descriptor_set_layout);
}

void EnvironmentMap::Reset() {
	m_hdr_image = nullptr;
	m_hdr_image_view = nullptr;
	m_alias_table_image = nullptr;
	m_alias_table_image_view = nullptr;
}

void EnvironmentMap::Reset(const std::shared_ptr<myvk::CommandPool> &command_pool, const char *filename) {
	HdrImg img = load_hdr_image(filename);
	if (!img.m_data) {
		Reset();
		return;
	}

	// TODO: calculate weights, copy to vulkan image
	std::vector<double> weights = weigh_hdr_image(&img);
	create_images(command_pool, img, &weights);

	free(img.m_data); // release hdr image data

	m_descriptor_set->UpdateCombinedImageSampler(m_sampler, m_hdr_image_view, 0);
	m_descriptor_set->UpdateCombinedImageSampler(m_sampler, m_alias_table_image_view, 1);
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
std::vector<double> EnvironmentMap::weigh_hdr_image(const EnvironmentMap::HdrImg *img) {
	const uint32_t img_size = img->m_width * img->m_height;

	std::vector<double> ret;
	ret.reserve(img_size);

	double weight_sum = 0.0;
	float *cur = img->m_data;
	for (uint32_t j = 0; j < img->m_height; ++j) {
		double sin_theta = glm::sin(M_PI * double(j) / double(img->m_height));
		for (uint32_t i = 0; i < img->m_width; ++i, cur += 4) {
			ret.push_back((double)luminance(cur[0], cur[1], cur[2]) * sin_theta);
			weight_sum += ret.back();
		}
	}

	// normalize weights, set as PDF
	for (uint32_t i = 0; i < img_size; ++i) {
		ret[i] *= (double)img_size / weight_sum;
		img->m_data[(i << 2) | 3] = (float)ret[i];
	}

	return ret;
}

void EnvironmentMap::generate_alias_table(std::vector<double> *weights_ptr, AliasPair *alias_table) {
	std::vector<double> &weights = *weights_ptr;
	const uint32_t n = weights.size();
	/* double weight_sum = 0.0;
	for (double &i : weights)
	    weight_sum += i;
	for (double &i : weights)
	    i *= (double)n / weight_sum; */
	std::vector<uint32_t> large_block, small_block;
	large_block.reserve(n);
	small_block.reserve(n);

	for (uint32_t i = 0; i < n; ++i) {
		if (weights[i] < 1.0)
			small_block.push_back(i);
		else
			large_block.push_back(i);
	}

	while (!small_block.empty() && !large_block.empty()) {
		uint32_t cur_small = small_block.back(), cur_large = large_block.back();
		small_block.pop_back();
		alias_table[cur_small].m_prob = (uint32_t)glm::clamp(weights[cur_small] * 4294967296.0, 0.0, 4294967295.0);
		alias_table[cur_small].m_alias = cur_large;
		weights[cur_large] -= 1.0 - weights[cur_small];
		if (weights[cur_large] < 1.0) {
			large_block.pop_back();
			small_block.push_back(cur_large);
		}
	}

	while (!large_block.empty()) {
		uint32_t cur = large_block.back();
		large_block.pop_back();
		alias_table[cur].m_prob = alias_table[cur].m_alias = 0xffffffffu;
	}

	while (!small_block.empty()) {
		uint32_t cur = small_block.back();
		small_block.pop_back();
		alias_table[cur].m_prob = alias_table[cur].m_alias = 0xffffffffu;
	}
}

void EnvironmentMap::create_images(const std::shared_ptr<myvk::CommandPool> &command_pool, const HdrImg &img,
                                   std::vector<double> *weights_ptr) {
	const uint32_t img_size = img.m_width * img.m_height;
	const VkDeviceSize img_bytes = img_size * sizeof(float) * 4;

	const std::shared_ptr<myvk::Device> &device = command_pool->GetDevicePtr();

	// create vulkan objects
	m_hdr_image = myvk::Image::CreateTexture2D(device, {img.m_width, img.m_height}, 1, VK_FORMAT_R32G32B32A32_SFLOAT,
	                                           VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	m_hdr_image_view = myvk::ImageView::Create(m_hdr_image, VK_IMAGE_VIEW_TYPE_2D);
	m_alias_table_image = myvk::Image::CreateTexture2D(device, {img.m_width, img.m_height}, 1, VK_FORMAT_R32G32_UINT,
	                                                   VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	m_alias_table_image_view = myvk::ImageView::Create(m_alias_table_image, VK_IMAGE_VIEW_TYPE_2D);

	// create and fill staging objects
	std::shared_ptr<myvk::Buffer> hdr_image_staging = myvk::Buffer::CreateStaging(device, img_bytes);
	hdr_image_staging->UpdateData(img.m_data, img.m_data + img_size * 4);

	std::shared_ptr<myvk::Buffer> alias_table_image_staging =
	    myvk::Buffer::CreateStaging(device, img_size * sizeof(AliasPair));
	{
		auto *data = (AliasPair *)alias_table_image_staging->Map();
		generate_alias_table(weights_ptr, data);
		alias_table_image_staging->Unmap();
	}

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
	command_buffer->CmdPipelineBarrier(
	    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {},
	    {m_hdr_image->GetDstMemoryBarrier(region, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
	                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
	     m_alias_table_image->GetDstMemoryBarrier(region, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
	                                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)});
	command_buffer->CmdCopy(hdr_image_staging, m_hdr_image, {region});
	command_buffer->CmdCopy(alias_table_image_staging, m_alias_table_image, {region});
	command_buffer->CmdPipelineBarrier(
	    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {}, {},
	    {
	        m_hdr_image->GetDstMemoryBarrier(region, VK_ACCESS_TRANSFER_WRITE_BIT, 0,
	                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
	        m_alias_table_image->GetDstMemoryBarrier(region, VK_ACCESS_TRANSFER_WRITE_BIT, 0,
	                                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
	    });
	command_buffer->End();

	command_buffer->Submit(fence);
	fence->Wait();
}

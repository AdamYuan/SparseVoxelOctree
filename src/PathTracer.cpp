#include "PathTracer.hpp"
#include "Config.hpp"
#include "Noise.inl"

inline static constexpr uint32_t group_8(uint32_t x) { return (x >> 3u) + ((x & 0x7u) ? 1u : 0u); }

void PathTracer::create_target_images(const std::shared_ptr<myvk::Device> &device,
                                      const std::vector<std::shared_ptr<myvk::Queue>> &access_queue) {
	m_color_image = myvk::Image::CreateTexture2D(
	    device, {m_width, m_height}, 1, VK_FORMAT_R32G32B32A32_SFLOAT,
	    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, access_queue);
	m_albedo_image = myvk::Image::CreateTexture2D(
	    device, {m_width, m_height}, 1, VK_FORMAT_R8G8B8A8_UNORM,
	    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, access_queue);
	m_normal_image = myvk::Image::CreateTexture2D(
	    device, {m_width, m_height}, 1, VK_FORMAT_R8G8B8A8_SNORM,
	    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, access_queue);

	m_color_image_view = myvk::ImageView::Create(m_color_image, VK_IMAGE_VIEW_TYPE_2D);
	m_albedo_image_view = myvk::ImageView::Create(m_albedo_image, VK_IMAGE_VIEW_TYPE_2D);
	m_normal_image_view = myvk::ImageView::Create(m_normal_image, VK_IMAGE_VIEW_TYPE_2D);
}

void PathTracer::create_noise_images(const std::shared_ptr<myvk::Device> &device) {
	m_noise_image = myvk::Image::CreateTexture2D(device, {kNoiseSize, kNoiseSize}, 1, VK_FORMAT_R8G8_UNORM,
	                                             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	m_noise_image_view = myvk::ImageView::Create(m_noise_image, VK_IMAGE_VIEW_TYPE_2D);
	m_noise_sampler = myvk::Sampler::Create(device, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
}

void PathTracer::create_descriptor(const std::shared_ptr<myvk::Device> &device) {
	m_descriptor_pool = myvk::DescriptorPool::Create(
	    device, 2, {{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}});
	{
		VkDescriptorSetLayoutBinding color_binding = {};
		color_binding.binding = 0;
		color_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		color_binding.descriptorCount = 1;
		color_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding albedo_binding = {};
		albedo_binding.binding = 1;
		albedo_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		albedo_binding.descriptorCount = 1;
		albedo_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding normal_binding = {};
		normal_binding.binding = 2;
		normal_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		normal_binding.descriptorCount = 1;
		normal_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		m_target_descriptor_set_layout =
		    myvk::DescriptorSetLayout::Create(device, {color_binding, albedo_binding, normal_binding});
	}

	{
		VkDescriptorSetLayoutBinding noise_binding = {};
		noise_binding.binding = 0;
		noise_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		noise_binding.descriptorCount = 1;
		noise_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		VkSampler immutable_samplers[] = {m_noise_sampler->GetHandle()};
		noise_binding.pImmutableSamplers = immutable_samplers;

		m_noise_descriptor_set_layout = myvk::DescriptorSetLayout::Create(device, {noise_binding});
	}
	m_target_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_target_descriptor_set_layout);

	m_noise_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_noise_descriptor_set_layout);
	m_noise_descriptor_set->UpdateCombinedImageSampler(m_noise_sampler, m_noise_image_view, 0);
}

void PathTracer::create_pipeline(const std::shared_ptr<myvk::Device> &device) {
	m_pipeline_layout = myvk::PipelineLayout::Create(
	    device,
	    {m_octree_ptr->GetDescriptorSetLayout(), m_camera_ptr->GetDescriptorSetLayout(),
	     m_lighting_ptr->GetEnvironmentMapPtr()->GetDescriptorSetLayout(), m_sobol.GetDescriptorSetLayout(),
	     m_target_descriptor_set_layout, m_noise_descriptor_set_layout},
	    {{VK_SHADER_STAGE_COMPUTE_BIT, 0, 2 * sizeof(uint32_t) + 5 * sizeof(float)}});
	{
		constexpr uint32_t kPathTracerCompSpv[] = {
#include "spirv/path_tracer.comp.u32"
		};
		std::shared_ptr<myvk::ShaderModule> path_tracer_shader_module =
		    myvk::ShaderModule::Create(device, kPathTracerCompSpv, sizeof(kPathTracerCompSpv));
		m_pipeline = myvk::ComputePipeline::Create(m_pipeline_layout, path_tracer_shader_module);
	}
}

void PathTracer::clear_target_images(const std::shared_ptr<myvk::CommandPool> &command_pool) {
	std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(command_pool);
	std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(command_pool->GetDevicePtr());
	command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	command_buffer->CmdPipelineBarrier(
	    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {},
	    {m_color_image->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
	                                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
	     m_albedo_image->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
	                                      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
	     m_normal_image->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
	                                      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)});

	command_buffer->CmdClearColorImage(m_color_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	command_buffer->CmdClearColorImage(m_albedo_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	command_buffer->CmdClearColorImage(m_normal_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	command_buffer->CmdPipelineBarrier(
	    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {}, {},
	    {m_color_image->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, 0,
	                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL),
	     m_albedo_image->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, 0,
	                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL),
	     m_normal_image->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, 0,
	                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL)});
	command_buffer->End();

	command_buffer->Submit(fence);
	fence->Wait();
}

void PathTracer::set_noise_image(const std::shared_ptr<myvk::CommandPool> &command_pool) {
	// create a staging buffer with maximum possible size
	std::shared_ptr<myvk::Buffer> staging_buffer =
	    myvk::Buffer::CreateStaging(command_pool->GetDevicePtr(), sizeof(kNoise));
	{
		uint8_t *data = (uint8_t *)staging_buffer->Map();
		std::copy(std::begin(kNoise), std::end(kNoise), data);
		staging_buffer->Unmap();
	}

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {kNoiseSize, kNoiseSize, 1};

	std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(command_pool);
	std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(command_pool->GetDevicePtr());
	command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	command_buffer->CmdPipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {},
	                                   m_noise_image->GetDstMemoryBarriers({region}, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
	                                                                       VK_IMAGE_LAYOUT_UNDEFINED,
	                                                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));
	command_buffer->CmdCopy(staging_buffer, m_noise_image, {region});
	command_buffer->CmdPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {}, {},
	                                   m_noise_image->GetDstMemoryBarriers({region}, VK_ACCESS_TRANSFER_WRITE_BIT, 0,
	                                                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                                                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
	command_buffer->End();

	command_buffer->Submit(fence);
	fence->Wait();
}

std::shared_ptr<PathTracer> PathTracer::Create(const std::shared_ptr<Octree> &octree,
                                               const std::shared_ptr<Camera> &camera,
                                               const std::shared_ptr<Lighting> &lighting,
                                               const std::shared_ptr<myvk::CommandPool> &command_pool) {
	std::shared_ptr<PathTracer> ret = std::make_shared<PathTracer>();
	ret->m_octree_ptr = octree;
	ret->m_camera_ptr = camera;
	ret->m_lighting_ptr = lighting;

	ret->m_bounce = kDefaultBounce;

	ret->m_sobol.Initialize(command_pool->GetDevicePtr());
	ret->create_noise_images(command_pool->GetDevicePtr());
	ret->set_noise_image(command_pool);
	ret->create_descriptor(command_pool->GetDevicePtr());
	ret->create_pipeline(command_pool->GetDevicePtr());

	return ret;
}

void PathTracer::Reset(const std::shared_ptr<myvk::CommandPool> &command_pool,
                       const std::shared_ptr<myvk::Queue> &shared_queue) {
	{
		float tmp = m_camera_ptr->m_aspect_ratio;
		m_camera_ptr->m_aspect_ratio = m_width / float(m_height);
		m_camera_ptr->UpdateFrameUniformBuffer(kFrameCount);
		m_camera_ptr->m_aspect_ratio = tmp;
	}
	m_sobol.Reset(command_pool, (m_bounce + 1) * 2);
	create_target_images(command_pool->GetDevicePtr(), {command_pool->GetQueuePtr(), shared_queue});
	clear_target_images(command_pool);
	m_target_descriptor_set->UpdateStorageImage(m_color_image_view, 0);
	m_target_descriptor_set->UpdateStorageImage(m_albedo_image_view, 1);
	m_target_descriptor_set->UpdateStorageImage(m_normal_image_view, 2);
}

void PathTracer::CmdRender(const std::shared_ptr<myvk::CommandBuffer> &command_buffer) {
	command_buffer->CmdBindDescriptorSets({m_octree_ptr->GetDescriptorSet(),
	                                       m_camera_ptr->GetFrameDescriptorSet(kFrameCount),
	                                       m_lighting_ptr->GetEnvironmentMapPtr()->GetDescriptorSet(),
	                                       m_sobol.GetDescriptorSet(), m_target_descriptor_set, m_noise_descriptor_set},
	                                      m_pipeline);

	uint32_t uint_push_constants[] = {m_bounce, (uint32_t)m_lighting_ptr->GetFinalLightType()};
	float float_push_constants[] = {
	    m_lighting_ptr->m_sun_radiance.x, m_lighting_ptr->m_sun_radiance.y, m_lighting_ptr->m_sun_radiance.z,
	    m_lighting_ptr->GetEnvironmentMapPtr()->m_rotation, m_lighting_ptr->GetEnvironmentMapPtr()->m_multiplier};

	command_buffer->CmdPushConstants(m_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint_push_constants),
	                                 uint_push_constants);
	command_buffer->CmdPushConstants(m_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(uint_push_constants),
	                                 sizeof(float_push_constants), float_push_constants);
	command_buffer->CmdBindPipeline(m_pipeline);
	command_buffer->CmdDispatch(group_8(m_width), group_8(m_height), 1);

	command_buffer->CmdPipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {},
	                                   {}, {});

	m_sobol.CmdNext(command_buffer);
}

void PathTracer::extract_target_image_to_buffer(const std::shared_ptr<myvk::CommandPool> &command_pool,
                                                const std::shared_ptr<myvk::ImageBase> &image,
                                                const std::shared_ptr<myvk::BufferBase> &buffer) {
	std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(command_pool);
	std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(command_pool->GetDevicePtr());
	command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VkBufferImageCopy region = {};
	region.imageExtent = image->GetExtent();
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	command_buffer->CmdPipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {},
	                                   image->GetDstMemoryBarriers({region}, 0, VK_ACCESS_TRANSFER_READ_BIT,
	                                                               VK_IMAGE_LAYOUT_GENERAL,
	                                                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL));
	command_buffer->CmdCopy(image, buffer, {region});
	command_buffer->CmdPipelineBarrier(
	    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {}, {},
	    image->GetDstMemoryBarriers({region}, 0, 0, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL));
	command_buffer->End();

	command_buffer->Submit(fence);
	fence->Wait();
}

std::vector<float> PathTracer::ExtractColorImage(const std::shared_ptr<myvk::CommandPool> &command_pool) const {
	const uint32_t kSize = m_width * m_height;
	std::shared_ptr<myvk::Buffer> staging_buffer =
	    myvk::Buffer::Create(command_pool->GetDevicePtr(), kSize * 4 * sizeof(float), VMA_MEMORY_USAGE_CPU_ONLY,
	                         VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	extract_target_image_to_buffer(command_pool, m_color_image, staging_buffer);

	auto *data = (float *)staging_buffer->Map();
	std::vector<float> pixels(kSize * 3);
	for (int i = 0; i < kSize; ++i) {
		pixels[i * 3 + 0] = data[i * 4 + 0];
		pixels[i * 3 + 1] = data[i * 4 + 1];
		pixels[i * 3 + 2] = data[i * 4 + 2];
	}
	staging_buffer->Unmap();

	return pixels;
}

std::vector<float> PathTracer::ExtractAlbedoImage(const std::shared_ptr<myvk::CommandPool> &command_pool) const {
	const uint32_t kSize = m_width * m_height;
	std::shared_ptr<myvk::Buffer> staging_buffer =
	    myvk::Buffer::Create(command_pool->GetDevicePtr(), kSize * sizeof(uint32_t), VMA_MEMORY_USAGE_CPU_ONLY,
	                         VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	extract_target_image_to_buffer(command_pool, m_albedo_image, staging_buffer);

	auto *data = (uint32_t *)staging_buffer->Map();
	std::vector<float> pixels(kSize * 3);
	for (int i = 0; i < kSize; ++i) {
		glm::vec3 v = glm::unpackUnorm4x8(data[i]);
		pixels[i * 3 + 0] = v.x;
		pixels[i * 3 + 1] = v.y;
		pixels[i * 3 + 2] = v.z;
	}
	staging_buffer->Unmap();

	return pixels;
}

std::vector<float> PathTracer::ExtractNormalImage(const std::shared_ptr<myvk::CommandPool> &command_pool) const {
	const uint32_t kSize = m_width * m_height;
	std::shared_ptr<myvk::Buffer> staging_buffer =
	    myvk::Buffer::Create(command_pool->GetDevicePtr(), kSize * sizeof(uint32_t), VMA_MEMORY_USAGE_CPU_ONLY,
	                         VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	extract_target_image_to_buffer(command_pool, m_normal_image, staging_buffer);

	auto *data = (uint32_t *)staging_buffer->Map();
	std::vector<float> pixels(kSize * 3);
	for (int i = 0; i < kSize; ++i) {
		glm::vec3 v = glm::unpackSnorm4x8(data[i]);
		pixels[i * 3 + 0] = v.x;
		pixels[i * 3 + 1] = v.y;
		pixels[i * 3 + 2] = v.z;
	}
	staging_buffer->Unmap();

	return pixels;
}

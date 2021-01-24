#include "PathTracer.hpp"
#include "Config.hpp"

void PathTracer::Initialize(const std::shared_ptr<myvk::Device> &device) {
	m_color_image = myvk::Image::CreateTexture2D(device, {kWidth, kHeight}, 1, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT);
	m_normal_image = myvk::Image::CreateTexture2D(device, {kWidth, kHeight}, 1, VK_FORMAT_R8G8B8A8_SNORM, VK_IMAGE_USAGE_STORAGE_BIT);
	m_albedo_image = myvk::Image::CreateTexture2D(device, {kWidth, kHeight}, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT);
}

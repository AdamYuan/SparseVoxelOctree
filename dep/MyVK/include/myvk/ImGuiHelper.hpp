#ifdef MYVK_ENABLE_IMGUI

#ifndef MYVK_IMGUI_HELPER_HPP
#define MYVK_IMGUI_HELPER_HPP

#include <imgui.h>
#include <imgui_impl_glfw.h>

#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "Image.hpp"
#include "ImageView.hpp"
#include "Sampler.hpp"

namespace myvk {

struct ImGuiInfo {
	Ptr<Image> font_texture;
	Ptr<ImageView> font_texture_view;
	Ptr<Sampler> font_texture_sampler;
};
extern ImGuiInfo kImGuiInfo;

template <typename FontLoaderFunc>
inline void ImGuiInit(GLFWwindow *window, const Ptr<CommandPool> &command_pool, FontLoaderFunc &&font_loader_func) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan(window, true);

	font_loader_func();

	unsigned char *data;
	int width, height;
	ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&data, &width, &height);
	uint32_t data_size = width * height * 4;

	kImGuiInfo.font_texture =
	    Image::CreateTexture2D(command_pool->GetDevicePtr(), {(uint32_t)width, (uint32_t)height}, 1,
	                           VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	ImGui::GetIO().Fonts->SetTexID((ImTextureID)(intptr_t)kImGuiInfo.font_texture->GetHandle());

	{
		Ptr<Fence> fence = Fence::Create(command_pool->GetDevicePtr());
		Ptr<Buffer> staging_buffer = Buffer::CreateStaging(command_pool->GetDevicePtr(), data, data + data_size);

		Ptr<CommandBuffer> command_buffer = CommandBuffer::Create(command_pool);
		command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = {0, 0, 0};
		region.imageExtent = {(uint32_t)width, (uint32_t)height, 1};

		command_buffer->CmdPipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {},
		                                   kImGuiInfo.font_texture->GetDstMemoryBarriers(
		                                       {region}, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
		                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));
		command_buffer->CmdCopy(staging_buffer, kImGuiInfo.font_texture, {region});
		command_buffer->CmdPipelineBarrier(
		    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, {}, {},
		    kImGuiInfo.font_texture->GetDstMemoryBarriers(
		        {region}, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

		command_buffer->End();

		command_buffer->Submit(fence);
		fence->Wait();
	}

	kImGuiInfo.font_texture_view = ImageView::Create(kImGuiInfo.font_texture, VK_IMAGE_VIEW_TYPE_2D,
	                                                 kImGuiInfo.font_texture->GetFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
	kImGuiInfo.font_texture_sampler =
	    Sampler::Create(command_pool->GetDevicePtr(), VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
}

inline void ImGuiInit(GLFWwindow *window, const Ptr<CommandPool> &command_pool) {
	ImGuiInit(window, command_pool, []() {});
}

inline void ImGuiNewFrame() {
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

} // namespace myvk

#endif // MYVK_IMGUIHELPER_HPP

#endif

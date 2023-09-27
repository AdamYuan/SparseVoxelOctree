#include "myvk/ImagelessFramebuffer.hpp"

namespace myvk {

Ptr<ImagelessFramebuffer>
ImagelessFramebuffer::Create(const Ptr<RenderPass> &render_pass,
                             const std::vector<VkFramebufferAttachmentImageInfo> &attachment_image_infos) {
	return Create(render_pass, attachment_image_infos,
	              {attachment_image_infos[0].width, attachment_image_infos[0].height}, 1);
}

Ptr<ImagelessFramebuffer> ImagelessFramebuffer::Create(const Ptr<myvk::RenderPass> &render_pass,
                                                       const std::vector<Ptr<ImageView>> &template_image_views) {
	uint32_t count = template_image_views.size();
	std::vector<VkFramebufferAttachmentImageInfo> infos(count);
	for (uint32_t i = 0; i < count; ++i)
		infos[i] = template_image_views[i]->GetFramebufferAttachmentImageInfo();
	return Create(render_pass, infos);
}

Ptr<ImagelessFramebuffer> ImagelessFramebuffer::Create(const Ptr<myvk::RenderPass> &render_pass,
                                                       const std::vector<Ptr<ImageBase>> &template_images) {
	uint32_t count = template_images.size();
	std::vector<VkFramebufferAttachmentImageInfo> infos(count);
	for (uint32_t i = 0; i < count; ++i)
		infos[i] = template_images[i]->GetFramebufferAttachmentImageInfo();
	return Create(render_pass, infos);
}

Ptr<ImagelessFramebuffer>
ImagelessFramebuffer::Create(const Ptr<RenderPass> &render_pass,
                             const std::vector<VkFramebufferAttachmentImageInfo> &attachment_image_infos,
                             const VkExtent2D &extent, uint32_t layers) {
	auto ret = std::make_shared<ImagelessFramebuffer>();
	ret->m_render_pass_ptr = render_pass;
	ret->m_extent = extent;
	ret->m_layers = layers;

	VkFramebufferCreateInfo create_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	create_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
	create_info.renderPass = render_pass->GetHandle();
	create_info.attachmentCount = attachment_image_infos.size();
	create_info.pAttachments = nullptr;
	create_info.width = extent.width;
	create_info.height = extent.height;
	create_info.layers = layers;

	VkFramebufferAttachmentsCreateInfo attachments_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO};
	attachments_info.attachmentImageInfoCount = attachment_image_infos.size();
	attachments_info.pAttachmentImageInfos = attachment_image_infos.data();

	create_info.pNext = &attachments_info;

	if (vkCreateFramebuffer(render_pass->GetDevicePtr()->GetHandle(), &create_info, nullptr, &ret->m_framebuffer) !=
	    VK_SUCCESS)
		return nullptr;

	return ret;
}

} // namespace myvk

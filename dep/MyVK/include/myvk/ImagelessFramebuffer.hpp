#ifndef MYVK_IMAGELESS_FRAMEBUFFER_HPP
#define MYVK_IMAGELESS_FRAMEBUFFER_HPP

#include "FramebufferBase.hpp"
#include "ImageView.hpp"

namespace myvk {
class ImagelessFramebuffer : public FramebufferBase {
private:
public:
	static Ptr<ImagelessFramebuffer> Create(const Ptr<RenderPass> &render_pass,
	                                        const std::vector<VkFramebufferAttachmentImageInfo> &attachment_image_infos,
	                                        const VkExtent2D &extent, uint32_t layers = 1);

	static Ptr<ImagelessFramebuffer>
	Create(const Ptr<RenderPass> &render_pass,
	       const std::vector<VkFramebufferAttachmentImageInfo> &attachment_image_infos);

	static Ptr<ImagelessFramebuffer> Create(const Ptr<RenderPass> &render_pass,
	                                        const std::vector<Ptr<ImageView>> &template_image_views);

	static Ptr<ImagelessFramebuffer> Create(const Ptr<RenderPass> &render_pass,
	                                        const std::vector<Ptr<ImageBase>> &template_images);

	~ImagelessFramebuffer() override = default;
};
} // namespace myvk

#endif

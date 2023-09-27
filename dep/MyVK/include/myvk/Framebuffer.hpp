#ifndef MYVK_FRAMEBUFFER_HPP
#define MYVK_FRAMEBUFFER_HPP

#include "FramebufferBase.hpp"
#include "ImageView.hpp"

#include <memory>
#include <vector>

namespace myvk {
class Framebuffer : public FramebufferBase {
private:
	std::vector<Ptr<ImageView>> m_image_view_ptrs;

public:
	static Ptr<Framebuffer> Create(const Ptr<RenderPass> &render_pass, const std::vector<Ptr<ImageView>> &image_views,
	                               const VkExtent2D &extent, uint32_t layers = 1);

	static Ptr<Framebuffer> Create(const Ptr<RenderPass> &render_pass, const Ptr<ImageView> &image_view);

	const std::vector<Ptr<ImageView>> &GetImageViewPtrs() const { return m_image_view_ptrs; }

	~Framebuffer() override = default;
};
} // namespace myvk

#endif

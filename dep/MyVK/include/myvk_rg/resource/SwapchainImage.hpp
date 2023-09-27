#ifndef MYVK_RG_SWAPCHAIN_IMAGE_HPP
#define MYVK_RG_SWAPCHAIN_IMAGE_HPP

#include <myvk_rg/RenderGraph.hpp>

namespace myvk_rg {
#ifdef MYVK_ENABLE_GLFW
class SwapchainImage final : public ExternalImageBase {
private:
	myvk::Ptr<myvk::FrameManager> m_frame_manager;

	MYVK_RG_OBJECT_FRIENDS
	inline void Initialize(const myvk::Ptr<myvk::FrameManager> &frame_manager) {
		m_frame_manager = frame_manager;
		SetDstLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

public:
	inline SwapchainImage() = default;
	inline SwapchainImage(SwapchainImage &&) noexcept = default;
	~SwapchainImage() final = default;
	inline const myvk::Ptr<myvk::ImageView> &GetVkImageView() const final {
		return m_frame_manager->GetCurrentSwapchainImageView();
	}
	inline bool IsStatic() const final { return false; }
};
#endif
} // namespace myvk_rg

#endif

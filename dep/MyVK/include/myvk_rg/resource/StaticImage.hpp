#ifndef MYVK_RG_STATIC_IMAGE_HPP
#define MYVK_RG_STATIC_IMAGE_HPP

#include <myvk_rg/RenderGraph.hpp>

namespace myvk_rg {
class StaticImage final : public myvk_rg::ExternalImageBase {
private:
	myvk::Ptr<myvk::ImageView> m_image_view;

	MYVK_RG_OBJECT_FRIENDS
	inline void Initialize(myvk::Ptr<myvk::ImageView> image_view, VkImageLayout layout) {
		m_image_view = std::move(image_view);
		SetSrcLayout(layout);
		SetDstLayout(layout);
	}

public:
	inline const myvk::Ptr<myvk::ImageView> &GetVkImageView() const final { return m_image_view; }
	inline bool IsStatic() const final { return true; }
};
} // namespace myvk_rg

#endif

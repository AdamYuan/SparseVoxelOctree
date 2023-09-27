#ifndef MYVK_RG_IMAGE_BLIT_PASS_HPP
#define MYVK_RG_IMAGE_BLIT_PASS_HPP

#include <myvk_rg/RenderGraph.hpp>

namespace myvk_rg {
class ImageBlitPass final : public myvk_rg::TransferPassBase {
private:
	myvk_rg::ImageInput m_src{}, m_dst{};
	VkFilter m_filter{};

	MYVK_RG_OBJECT_FRIENDS
	inline void Initialize(myvk_rg::ImageInput src, myvk_rg::ImageInput dst, VkFilter filter) {
		m_src = src, m_dst = dst;
		m_filter = filter;
		AddInput<myvk_rg::Usage::kTransferImageSrc, VK_PIPELINE_STAGE_2_BLIT_BIT>({"src"}, src);
		AddInput<myvk_rg::Usage::kTransferImageDst, VK_PIPELINE_STAGE_2_BLIT_BIT>({"dst"}, dst);
	}

public:
	inline void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final {
		command_buffer->CmdBlitImage(m_src->GetVkImageView(), m_dst->GetVkImageView(), m_filter);
	}
	inline auto GetDstOutput() { return MakeImageOutput({"dst"}); }
};
} // namespace myvk_rg

#endif

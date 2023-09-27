#ifndef MYVK_RG_STATIC_BUFFER_HPP
#define MYVK_RG_STATIC_BUFFER_HPP

#include <myvk_rg/RenderGraph.hpp>

namespace myvk_rg {
template <typename Buffer = myvk::Buffer> class StaticBuffer final : public myvk_rg::ExternalBufferBase {
	static_assert(std::is_base_of_v<myvk::BufferBase, Buffer>);

private:
	myvk::Ptr<Buffer> m_buffer;
	myvk::Ptr<myvk::BufferBase> m_buffer_base;

	MYVK_RG_OBJECT_FRIENDS
	inline void Initialize(myvk::Ptr<Buffer> buffer) {
		m_buffer = std::move(buffer);
		m_buffer_base = m_buffer;
	}

public:
	inline const auto &GetBuffer() const { return m_buffer; }

	inline const myvk::Ptr<myvk::BufferBase> &GetVkBuffer() const final { return m_buffer_base; }
	inline bool IsStatic() const final { return true; }
};
} // namespace myvk_rg

#endif

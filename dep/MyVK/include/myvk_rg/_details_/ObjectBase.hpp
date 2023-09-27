#ifndef MYVK_RG_BASE_HPP
#define MYVK_RG_BASE_HPP

namespace myvk_rg::_details_ {

class RenderGraphBase;
class PassBase;
class ResourceBase;
// Object Base
class PoolKey;
class ObjectBase {
private:
	RenderGraphBase *m_render_graph_ptr{};
	const PoolKey *m_key_ptr{};

	inline void set_render_graph_ptr(RenderGraphBase *render_graph_ptr) { m_render_graph_ptr = render_graph_ptr; }
	inline void set_key_ptr(const PoolKey *key_ptr) { m_key_ptr = key_ptr; }

	template <typename, typename...> friend class Pool;

public:
	inline ObjectBase() = default;
	inline virtual ~ObjectBase() = default;

	inline RenderGraphBase *GetRenderGraphPtr() const { return m_render_graph_ptr; }
	inline const PoolKey &GetKey() const { return *m_key_ptr; }

	// Disable Copy
	inline ObjectBase(ObjectBase &&r) noexcept = default;
	// inline ObjectBase &operator=(ObjectBase &&r) noexcept = default;
};

} // namespace myvk_rg::_details_

#endif

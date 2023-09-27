#ifndef MYVK_RG_DETAILS_RENDER_GRAPH_HPP
#define MYVK_RG_DETAILS_RENDER_GRAPH_HPP

#include "Pass.hpp"
#include "RenderGraphBase.hpp"
#include "Resource.hpp"

namespace myvk_rg::_details_ {

template <typename Derived>
class RenderGraph : public RenderGraphBase,
                    public PassPool<Derived>,
                    public ResourcePool<Derived>,
                    public Pool<Derived, const ResourceBase *> {
private:
	using _ResultPool = Pool<Derived, const ResourceBase *>;

protected:
	inline bool AddResult(const PoolKey &result_key, const ImageAlias *resource) {
		assert(resource);
		SetCompilePhrases(CompilePhrase::kResolve);
		return _ResultPool::template CreateAndInitializeForce<0, const ResourceBase *>(result_key, resource);
	}
	inline bool AddResult(const PoolKey &result_key, const BufferAlias *resource) {
		assert(resource);
		SetCompilePhrases(CompilePhrase::kResolve);
		return _ResultPool::template CreateAndInitializeForce<0, const ResourceBase *>(result_key, resource);
	}
	inline bool IsResultExist(const PoolKey &result_key) const { return _ResultPool::Exist(result_key); }
	inline void RemoveResult(const PoolKey &result_key) {
		SetCompilePhrases(CompilePhrase::kResolve);
		_ResultPool::Delete(result_key);
	}
	inline void ClearResults() {
		SetCompilePhrases(CompilePhrase::kResolve);
		_ResultPool::Clear();
	}

public:
	inline ~RenderGraph() override = default;

	template <typename... Args>
	inline static myvk::Ptr<Derived> Create(const myvk::Ptr<myvk::Queue> &main_queue_ptr, Args &&...args) {
		static_assert(std::is_base_of_v<RenderGraph<Derived>, Derived>);

		auto ret = std::make_shared<Derived>();
		dynamic_cast<RenderGraphBase *>(ret.get())->Initialize(main_queue_ptr);
		ret->Initialize(std::forward<Args>(args)...);
		return ret;
	}
	inline RenderGraph() { m_p_result_pool_data = &_ResultPool::GetPoolData(); }
};

} // namespace myvk_rg::_details_

#endif

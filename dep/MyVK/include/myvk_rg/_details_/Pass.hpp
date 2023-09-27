#ifndef MYVK_RG_PASS_HPP
#define MYVK_RG_PASS_HPP

#include "Input.hpp"
#include "RenderGraphBase.hpp"
#include "Resource.hpp"

#include <myvk/CommandBuffer.hpp>
#include <myvk/ComputePipeline.hpp>
#include <myvk/GraphicsPipeline.hpp>

namespace myvk_rg::_details_ {

enum class PassType : uint8_t { kGraphics, kCompute, kTransfer, kGroup };

class GraphicsPassBase;
class ComputePassBase;
class TransferPassBase;
class PassGroupBase;

class PassBase : public ObjectBase {
private:
	PassType m_type{};

	// Pass
	const _details_rg_pool_::InputPoolData *m_p_input_pool_data{};
	const DescriptorSetData *m_p_descriptor_set_data{};
	// GraphicsPass
	const AttachmentData *m_p_attachment_data{};

	mutable struct {
	private:
		uint32_t pass_order{};
		friend class RenderGraphResolver;
	} m_resolved_info{};

	mutable struct {
	private:
		uint32_t pass_id{}, subpass_id{};
		friend class RenderGraphScheduler;
	} m_scheduled_info{};

	mutable struct {
	private:
		bool pipeline_updated{};
		friend class RenderGraphExecutor;

		friend class GraphicsPassBase;
		friend class ComputePassBase;
	} m_executor_info;

	friend class PassGroupBase;
	friend class GraphicsPassBase;
	friend class ComputePassBase;
	friend class TransferPassBase;

	friend class RenderGraphBase;
	friend class RenderGraphResolver;
	friend class RenderGraphScheduler;
	friend class RenderGraphExecutor;
	friend class RenderGraphDescriptor;

	template <typename Func> inline void for_each_input(Func &&func) {
		for (auto it = m_p_input_pool_data->pool.begin(); it != m_p_input_pool_data->pool.end(); ++it)
			func(*(m_p_input_pool_data->ValueGet<0, Input>(it)));
	}
	template <typename Func> inline void for_each_input(Func &&func) const {
		for (auto it = m_p_input_pool_data->pool.begin(); it != m_p_input_pool_data->pool.end(); ++it)
			func(m_p_input_pool_data->ValueGet<0, Input>(it));
	}

public:
	inline PassBase(PassType type) : m_type{type} {};
	inline PassBase(PassBase &&) noexcept = default;
	inline ~PassBase() override = default;

	inline PassType GetType() const { return m_type; }

	template <typename Visitor> inline std::invoke_result_t<Visitor, GraphicsPassBase *> Visit(Visitor &&visitor);
	template <typename Visitor> inline std::invoke_result_t<Visitor, GraphicsPassBase *> Visit(Visitor &&visitor) const;

	virtual void CreatePipeline() = 0;
	virtual void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const = 0;
};

template <typename Derived> class PassPool : public Pool<Derived, PassBase> {
private:
	using _PassPool = Pool<Derived, PassBase>;

	inline RenderGraphBase *get_render_graph_ptr() {
		static_assert(std::is_base_of_v<ObjectBase, Derived> || std::is_base_of_v<RenderGraphBase, Derived>);
		if constexpr (std::is_base_of_v<ObjectBase, Derived>)
			return static_cast<ObjectBase *>(static_cast<Derived *>(this))->GetRenderGraphPtr();
		else
			return static_cast<RenderGraphBase *>(static_cast<Derived *>(this));
	}

public:
	inline PassPool() = default;
	inline PassPool(PassPool &&) noexcept = default;
	inline ~PassPool() override = default;

protected:
	template <typename PassType, typename... Args, typename = std::enable_if_t<std::is_base_of_v<PassBase, PassType>>>
	inline PassType *CreatePass(const PoolKey &pass_key, Args &&...args) {
		PassType *ret =
		    _PassPool::template CreateAndInitialize<0, PassType, Args...>(pass_key, std::forward<Args>(args)...);
		assert(ret);
		get_render_graph_ptr()->SetCompilePhrases(CompilePhrase::kResolve);
		return ret;
	}
	inline void DeletePass(const PoolKey &pass_key) { return PassPool::Delete(pass_key); }

	template <typename PassType = PassBase,
	          typename = std::enable_if_t<std::is_base_of_v<PassBase, PassType> || std::is_same_v<PassBase, PassType>>>
	inline PassType *GetPass(const PoolKey &pass_key) const {
		return _PassPool::template Get<0, PassType>(pass_key);
	}
	inline void ClearPasses() {
		_PassPool::Clear();
		get_render_graph_ptr()->SetCompilePhrases(CompilePhrase::kResolve);
	}
};

class GraphicsPassBase : public PassBase,
                         public InputPool<GraphicsPassBase>,
                         public ResourcePool<GraphicsPassBase>,
                         public AttachmentInputSlot<GraphicsPassBase>,
                         public DescriptorInputSlot<GraphicsPassBase> {
public:
	inline constexpr PassType GetType() const { return PassType::kGraphics; }

	inline GraphicsPassBase() : PassBase(PassType::kGraphics) {
		m_p_input_pool_data = &InputPool<GraphicsPassBase>::GetPoolData();
		m_p_descriptor_set_data = &DescriptorInputSlot<GraphicsPassBase>::GetDescriptorSetData();
		m_p_attachment_data = &AttachmentInputSlot<GraphicsPassBase>::GetAttachmentData();
	}
	inline GraphicsPassBase(GraphicsPassBase &&) noexcept = default;
	inline ~GraphicsPassBase() override = default;

	uint32_t GetSubpass() const;
	const myvk::Ptr<myvk::RenderPass> &GetVkRenderPass() const;

	inline void UpdatePipeline() const { m_executor_info.pipeline_updated = true; }
};

class ComputePassBase : public PassBase,
                        public InputPool<ComputePassBase>,
                        public ResourcePool<ComputePassBase>,
                        public DescriptorInputSlot<ComputePassBase> {
public:
	inline constexpr PassType GetType() const { return PassType::kCompute; }

	inline ComputePassBase() : PassBase(PassType::kCompute) {
		m_p_input_pool_data = &InputPool<ComputePassBase>::GetPoolData();
		m_p_descriptor_set_data = &DescriptorInputSlot<ComputePassBase>::GetDescriptorSetData();
	}
	inline ComputePassBase(ComputePassBase &&) noexcept = default;
	inline ~ComputePassBase() override = default;

	inline void UpdatePipeline() const { m_executor_info.pipeline_updated = true; }
};

class TransferPassBase : public PassBase, public InputPool<TransferPassBase>, public ResourcePool<TransferPassBase> {
public:
	inline constexpr PassType GetType() const { return PassType::kTransfer; }

	inline TransferPassBase() : PassBase(PassType::kTransfer) {
		m_p_input_pool_data = &InputPool<TransferPassBase>::GetPoolData();
	}
	inline TransferPassBase(TransferPassBase &&) noexcept = default;
	inline ~TransferPassBase() override = default;

	inline void CreatePipeline() final {}
};

class PassGroupBase : public PassBase,
                      public ResourcePool<PassGroupBase>,
                      public PassPool<PassGroupBase>,
                      public AliasOutputPool<PassGroupBase> {
public:
	inline constexpr PassType GetType() const { return PassType::kGroup; }

	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &) const final {}

	inline PassGroupBase() : PassBase(PassType::kGroup) {}
	inline PassGroupBase(PassGroupBase &&) noexcept = default;
	inline ~PassGroupBase() override = default;

	inline void CreatePipeline() final {}
};

template <typename Visitor> std::invoke_result_t<Visitor, GraphicsPassBase *> PassBase::Visit(Visitor &&visitor) {
	switch (GetType()) {
	case PassType::kGraphics:
		return visitor(static_cast<GraphicsPassBase *>(this));
	case PassType::kCompute:
		return visitor(static_cast<ComputePassBase *>(this));
	case PassType::kTransfer:
		return visitor(static_cast<TransferPassBase *>(this));
	case PassType::kGroup:
		return visitor(static_cast<PassGroupBase *>(this));
	default:
		assert(false);
	}
	return visitor(static_cast<GraphicsPassBase *>(nullptr));
}

template <typename Visitor> std::invoke_result_t<Visitor, GraphicsPassBase *> PassBase::Visit(Visitor &&visitor) const {
	switch (GetType()) {
	case PassType::kGraphics:
		return visitor(static_cast<const GraphicsPassBase *>(this));
	case PassType::kCompute:
		return visitor(static_cast<const ComputePassBase *>(this));
	case PassType::kTransfer:
		return visitor(static_cast<const TransferPassBase *>(this));
	case PassType::kGroup:
		return visitor(static_cast<const PassGroupBase *>(this));
	default:
		assert(false);
	}
	return visitor(static_cast<const GraphicsPassBase *>(nullptr));
}

} // namespace myvk_rg::_details_

#endif

#ifndef MYVK_RG_RENDER_GRAPH_BASE_HPP
#define MYVK_RG_RENDER_GRAPH_BASE_HPP

#include <myvk/CommandBuffer.hpp>
#include <myvk/DescriptorSet.hpp>
#include <myvk/DeviceObjectBase.hpp>

#include <list>
#include <memory>
#include <unordered_set>

#include "Usage.hpp"

namespace myvk_rg {
enum class CompilePhrase : uint8_t {
	kResolve = 1u,
	kCreateDescriptor = 2u,
	kSchedule = 4u,
	kAllocate = 8u,
	kPrepareExecutor = 16u,
	kPreBindDescriptor = 32u,
	kInitLastFrameResource = 64u
};

inline constexpr CompilePhrase operator|(CompilePhrase x, CompilePhrase y) {
	return static_cast<CompilePhrase>(static_cast<uint8_t>(x) | static_cast<uint8_t>(y));
}
} // namespace myvk_rg

namespace myvk_rg::_details_ {

class PassBase;
namespace _details_rg_pool_ {
using ResultPoolData = PoolData<const ResourceBase *>;
}
class ImageBase;
class ManagedBuffer;
class CombinedImage;

class RenderGraphAllocation;

class RenderGraphBase : public myvk::DeviceObjectBase {
public:
	inline void SetCompilePhrases(CompilePhrase phrase) const { m_compile_phrase |= static_cast<uint8_t>(phrase); }

private:
	myvk::Ptr<myvk::Queue> m_main_queue_ptr;

	const _details_rg_pool_::ResultPoolData *m_p_result_pool_data{};

	VkExtent2D m_canvas_size{};
	// bool m_lazy_allocation_supported{};

	mutable uint8_t m_compile_phrase{};

	struct Compiler;
	std::unique_ptr<Compiler> m_compiler{};
	mutable bool m_exe_flip{};

	void Initialize(const myvk::Ptr<myvk::Queue> &main_queue);

	void compile() const;

	template <typename> friend class RenderGraph;

	friend class ManagedBuffer;
	friend class LastFrameBuffer;

	friend class ManagedImage;
	friend class CombinedImage;
	friend class LastFrameImage;

	friend class DescriptorSetData;
	friend class GraphicsPassBase;

	friend class RenderGraphResolver;
	friend class RenderGraphExecutor;

public:
	RenderGraphBase();
	~RenderGraphBase() override;

	inline void SetCanvasSize(const VkExtent2D &canvas_size) {
		if (canvas_size.width != m_canvas_size.width || canvas_size.height != m_canvas_size.height) {
			m_canvas_size = canvas_size;
			SetCompilePhrases(CompilePhrase::kAllocate | CompilePhrase::kSchedule);
		}
	}
	inline const VkExtent2D &GetCanvasSize() const { return m_canvas_size; }

	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const;

	inline const myvk::Ptr<myvk::Device> &GetDevicePtr() const final { return m_main_queue_ptr->GetDevicePtr(); }
	inline const auto &GetMainQueuePtr() const { return m_main_queue_ptr; }
};

} // namespace myvk_rg::_details_

#endif

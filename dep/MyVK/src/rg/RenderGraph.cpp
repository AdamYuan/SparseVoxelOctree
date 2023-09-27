#include <myvk_rg/RenderGraph.hpp>

#include "RenderGraphAllocator.hpp"
#include "RenderGraphDescriptor.hpp"
#include "RenderGraphExecutor.hpp"
#include "RenderGraphLFInit.hpp"
#include "RenderGraphResolver.hpp"
#include "RenderGraphScheduler.hpp"

#include "myvk_rg/_details_/Input.hpp"
#include "myvk_rg/_details_/Pass.hpp"
#include "myvk_rg/_details_/RenderGraphBase.hpp"
#include "myvk_rg/_details_/Resource.hpp"

namespace myvk_rg::_details_ {

struct RenderGraphBase::Compiler {
	RenderGraphResolver resolver;
	RenderGraphDescriptor descriptor;
	RenderGraphScheduler scheduler;
	RenderGraphAllocator allocator;
	RenderGraphExecutor executor;
	RenderGraphLFInit lf_init;
};

void RenderGraphBase::Initialize(const myvk::Ptr<myvk::Queue> &main_queue) {
	m_main_queue_ptr = main_queue;
	// Check Lazy Allocation Support
	/* for (uint32_t i = 0; i < GetDevicePtr()->GetPhysicalDevicePtr()->GetMemoryProperties().memoryTypeCount; i++) {
	    if (GetDevicePtr()->GetPhysicalDevicePtr()->GetMemoryProperties().memoryTypes[i].propertyFlags &
	        VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
	        m_lazy_allocation_supported = true;
	        break;
	    }
	} */
	// Create Compiler
	m_compiler = std::make_unique<Compiler>();
}

RenderGraphBase::RenderGraphBase() = default;
RenderGraphBase::~RenderGraphBase() = default;

void RenderGraphBase::compile() const {
#define CAST8(x) static_cast<uint8_t>(x)
	if (m_compile_phrase == 0u)
		return;

	/*
	 * The RenderGraph Compile Phrases
	 *
	 *            /-----> Schedule ------\
	 *           |                       |--> Prepare Executor
	 * Resolve --|------> Allocate -----<|--> LastFrame Init
	 *           |                       |--> Pre-bind Descriptor
	 *            \-> Create Descriptor -/
	 */

	uint8_t exe_compile_phrase = m_compile_phrase;
	if (m_compile_phrase & CAST8(CompilePhrase::kResolve))
		exe_compile_phrase |= CAST8(CompilePhrase::kSchedule | CompilePhrase::kCreateDescriptor |
		                            CompilePhrase::kAllocate | CompilePhrase::kPrepareExecutor |
		                            CompilePhrase::kPreBindDescriptor | CompilePhrase::kInitLastFrameResource);
	if (m_compile_phrase & CAST8(CompilePhrase::kAllocate))
		exe_compile_phrase |= CAST8(CompilePhrase::kPrepareExecutor | CompilePhrase::kPreBindDescriptor |
		                            CompilePhrase::kInitLastFrameResource);
	if (m_compile_phrase & CAST8(CompilePhrase::kSchedule))
		exe_compile_phrase |= CAST8(CompilePhrase::kPrepareExecutor);
	if (m_compile_phrase & CAST8(CompilePhrase::kCreateDescriptor))
		exe_compile_phrase |= CAST8(CompilePhrase::kPreBindDescriptor);
	m_compile_phrase = 0u;

	if (exe_compile_phrase & CAST8(CompilePhrase::kResolve))
		m_compiler->resolver.Resolve(this);
	if (exe_compile_phrase & CAST8(CompilePhrase::kCreateDescriptor))
		m_compiler->descriptor.Create(GetDevicePtr(), m_compiler->resolver);
	if (exe_compile_phrase & CAST8(CompilePhrase::kSchedule))
		m_compiler->scheduler.Schedule(m_compiler->resolver);
	if (exe_compile_phrase & CAST8(CompilePhrase::kAllocate))
		m_compiler->allocator.Allocate(GetDevicePtr(), m_compiler->resolver);
	if (exe_compile_phrase & CAST8(CompilePhrase::kPrepareExecutor))
		m_compiler->executor.Prepare(GetDevicePtr(), m_compiler->resolver, m_compiler->scheduler,
		                             m_compiler->allocator);
	if (exe_compile_phrase & CAST8(CompilePhrase::kPreBindDescriptor))
		m_compiler->descriptor.PreBind(m_compiler->allocator);
	if (exe_compile_phrase & CAST8(CompilePhrase::kInitLastFrameResource))
		m_compiler->lf_init.InitLastFrameResources(m_main_queue_ptr, m_compiler->allocator);
#undef CAST8
}

void RenderGraphBase::CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const {
	compile(); // Check & Compile before every execution
	m_compiler->descriptor.ExecutionBind(m_exe_flip);
	m_compiler->executor.CmdExecute(command_buffer, m_exe_flip);
	m_exe_flip ^= 1u;
}

// Resource GetVk functions
const myvk::Ptr<myvk::BufferBase> &ManagedBuffer::GetVkBuffer() const {
	return GetRenderGraphPtr()->m_compiler->allocator.GetVkBuffer(this, GetRenderGraphPtr()->m_exe_flip);
}
const myvk::Ptr<myvk::BufferBase> &LastFrameBuffer::GetVkBuffer() const {
	return GetRenderGraphPtr()->m_compiler->allocator.GetVkBuffer(this, !GetRenderGraphPtr()->m_exe_flip);
}

const myvk::Ptr<myvk::ImageView> &ManagedImage::GetVkImageView() const {
	return GetRenderGraphPtr()->m_compiler->allocator.GetVkImageView(this, GetRenderGraphPtr()->m_exe_flip);
}
const myvk::Ptr<myvk::ImageView> &CombinedImage::GetVkImageView() const {
	return GetRenderGraphPtr()->m_compiler->allocator.GetVkImageView(this, GetRenderGraphPtr()->m_exe_flip);
}
const myvk::Ptr<myvk::ImageView> &LastFrameImage::GetVkImageView() const {
	return GetRenderGraphPtr()->m_compiler->allocator.GetVkImageView(this, !GetRenderGraphPtr()->m_exe_flip);
}

void *ManagedBuffer::get_mapped_data() const {
	return GetRenderGraphPtr()->m_compiler->allocator.GetMappedData(this, GetRenderGraphPtr()->m_exe_flip);
}

// Descriptor GetVk functions
const myvk::Ptr<myvk::DescriptorSetLayout> &DescriptorSetData::GetVkDescriptorSetLayout(const PassBase *pass) const {
	const auto *render_graph = pass->GetRenderGraphPtr();
	return render_graph->m_compiler->descriptor.GetVkDescriptorSet(pass, false)->GetDescriptorSetLayoutPtr();
}
const myvk::Ptr<myvk::DescriptorSet> &DescriptorSetData::GetVkDescriptorSet(const PassBase *pass) const {
	const auto *render_graph = pass->GetRenderGraphPtr();
	return render_graph->m_compiler->descriptor.GetVkDescriptorSet(pass, render_graph->m_exe_flip);
}

// RenderPass GetVk functions
const myvk::Ptr<myvk::RenderPass> &GraphicsPassBase::GetVkRenderPass() const {
	return GetRenderGraphPtr()->m_compiler->executor.GetPassExec(this).render_pass_info.myvk_render_pass;
}
uint32_t GraphicsPassBase::GetSubpass() const { return RenderGraphScheduler::GetSubpassID(this); }

} // namespace myvk_rg::_details_

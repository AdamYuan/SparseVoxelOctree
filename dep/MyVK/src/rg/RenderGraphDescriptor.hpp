#ifndef MYVK_RG_RENDER_GRAPH_DESCRIPTOR_HPP
#define MYVK_RG_RENDER_GRAPH_DESCRIPTOR_HPP

#include "RenderGraphResolver.hpp"

#include "RenderGraphAllocator.hpp"

#include <myvk/DescriptorPool.hpp>

namespace myvk_rg::_details_ {

class RenderGraphDescriptor {
private:
	template <typename Resource> struct DescriptorBinding {
		const Resource *resource{};
		const Input *p_input{};
	};
	template <typename Resource> using DescriptorBindingMap = std::unordered_map<uint32_t, DescriptorBinding<Resource>>;
	struct PassDescriptor {
		DescriptorBindingMap<InternalImageBase> int_image_bindings;
		DescriptorBindingMap<LastFrameImage> lf_image_bindings;
		DescriptorBindingMap<ExternalImageBase> static_ext_image_bindings;
		DescriptorBindingMap<ExternalImageBase> dynamic_ext_image_bindings;

		DescriptorBindingMap<ManagedBuffer> int_buffer_bindings;
		DescriptorBindingMap<LastFrameBuffer> lf_buffer_bindings;
		DescriptorBindingMap<ExternalBufferBase> static_ext_buffer_bindings;
		DescriptorBindingMap<ExternalBufferBase> dynamic_ext_buffer_bindings;

		myvk::Ptr<myvk::DescriptorSet> sets[2]{};
	};
	std::vector<PassDescriptor> m_pass_descriptors;
	myvk::Ptr<myvk::Device> m_device_ptr;

public:
	void Create(const myvk::Ptr<myvk::Device> &device, const RenderGraphResolver &resolved);
	void PreBind(const RenderGraphAllocator &flip);
	void ExecutionBind(bool flip);
	inline const auto &GetVkDescriptorSet(uint32_t pass_order, bool db) const {
		return m_pass_descriptors[pass_order].sets[db];
	}
	inline const auto &GetVkDescriptorSet(const PassBase *pass, bool db) const {
		return m_pass_descriptors[RenderGraphResolver::GetPassOrder(pass)].sets[db];
	}
};

} // namespace myvk_rg::_details_

#endif

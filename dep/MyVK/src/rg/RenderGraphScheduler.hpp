#ifndef MYVK_RG_RENDER_GRAPH_SCHEDULER_HPP
#define MYVK_RG_RENDER_GRAPH_SCHEDULER_HPP

#include "RenderGraphResolver.hpp"

#include <span>

namespace myvk_rg::_details_ {

class RenderGraphScheduler {
public:
	struct SubpassDependency {
		const ResourceBase *resource{};
		ResourceReference from{}, to{};
	};
	struct SubpassInfo {
		const PassBase *pass{};
	};
	struct PassDependency {
		const ResourceBase *resource{};
		std::vector<ResourceReference> from, to;
		DependencyType type{};
	};
	struct RenderPassArea {
		VkExtent2D extent{};
		uint32_t layers{};
		inline bool operator==(const RenderPassArea &r) const {
			return std::tie(extent.width, extent.height, layers) == std::tie(r.extent.width, r.extent.height, r.layers);
		}
	};
	struct RenderPassInfo {
		std::vector<SubpassDependency> subpass_dependencies;
		std::unordered_map<const ImageBase *, uint32_t> attachment_id_map;
		RenderPassArea area;
	};
	struct PassInfo {
		std::vector<SubpassInfo> subpasses;
		RenderPassInfo *p_render_pass_info{};
	};

private:
	std::vector<PassInfo> m_passes;
	std::vector<RenderPassInfo> m_render_passes;
	std::vector<PassDependency> m_pass_dependencies;

	struct RenderPassMergeInfo;
	static std::vector<RenderPassMergeInfo> _compute_pass_merge_info(const RenderGraphResolver &resolved);

	void extract_grouped_passes(const RenderGraphResolver &resolved);
	void extract_dependencies(const RenderGraphResolver &resolved);
	void sort_and_insert_image_dependencies();
	void extract_pass_attachments();
	void extract_resource_transient_info();

	inline static std::span<const ResourceReference>
	get_last_image_references(const std::vector<ResourceReference> &last_references) {
		VkImageLayout last_layout{VK_IMAGE_LAYOUT_UNDEFINED};
		uint32_t i = last_references.size() - 1;
		for (; ~i; --i) {
			auto usage = last_references[i].p_input->GetUsage();
			if (last_layout != VK_IMAGE_LAYOUT_UNDEFINED && UsageGetImageLayout(usage) != last_layout)
				return std::span{last_references}.subspan(i + 1u);
			if (UsageIsAttachment(usage) || !UsageIsReadOnly(usage))
				return std::span{last_references}.subspan(i);
			last_layout = UsageGetImageLayout(usage);
		}
		return last_references;
	}

public:
	void Schedule(const RenderGraphResolver &resolved);

	inline uint32_t GetPassCount() const { return m_passes.size(); }
	inline const PassInfo &GetPassInfo(uint32_t pass_id) const { return m_passes[pass_id]; }
	inline static uint32_t GetPassID(const PassBase *pass) { return pass->m_scheduled_info.pass_id; }
	inline static uint32_t GetSubpassID(const PassBase *pass) { return pass->m_scheduled_info.subpass_id; }

	inline const std::vector<PassInfo> &GetPassInfos() const { return m_passes; }
	inline const std::vector<PassDependency> &GetPassDependencies() const { return m_pass_dependencies; }

	template <ResourceType ResType>
	inline static std::span<const ResourceReference>
	GetLastReferences(const std::vector<ResourceReference> &last_references) {
		if constexpr (ResType == ResourceType::kBuffer)
			return last_references;
		else
			return get_last_image_references(last_references);
	}

	inline static std::span<const ResourceReference>
	GetLastReferences(ResourceType resource_type, const std::vector<ResourceReference> &last_references) {
		if (resource_type == ResourceType::kBuffer)
			return last_references;
		else
			return get_last_image_references(last_references);
	}
};

} // namespace myvk_rg::_details_

#endif

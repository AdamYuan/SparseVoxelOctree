#ifndef MYVK_RENDER_PASS_HPP
#define MYVK_RENDER_PASS_HPP

#include "DeviceObjectBase.hpp"
#include "volk.h"
#include <memory>

#include <map>
#include <string>

namespace myvk {

class RenderPassState;
class RenderPass : public DeviceObjectBase {
private:
	Ptr<Device> m_device_ptr;
	VkRenderPass m_render_pass{VK_NULL_HANDLE};

public:
	static Ptr<RenderPass> Create(const Ptr<Device> &device, const VkRenderPassCreateInfo &create_info);
	static Ptr<RenderPass> Create(const Ptr<Device> &device, const VkRenderPassCreateInfo2 &create_info);
	static Ptr<RenderPass> Create(const Ptr<Device> &device, const RenderPassState &state);

	VkRenderPass GetHandle() const { return m_render_pass; }

	const Ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	~RenderPass() override;
};

class RenderPassState {
public:
	class SubpassAttachmentHandle {
	private:
		RenderPassState *m_state_ptr;
		uint32_t m_subpass;

	public:
		SubpassAttachmentHandle(RenderPassState *state_ptr, uint32_t subpass)
		    : m_state_ptr{state_ptr}, m_subpass{subpass} {}
		SubpassAttachmentHandle
		AddColorResolveAttachment(const char *attachment_str, VkImageLayout layout, const char *resolve_attachment_str,
		                          VkImageLayout resolve_layout, const char *generator_subpass_str,
		                          VkPipelineStageFlags generator_stage, VkPipelineStageFlags use_stage,
		                          VkAccessFlags generator_access, VkAccessFlags use_access,
		                          VkDependencyFlags dependency_flag = VK_DEPENDENCY_BY_REGION_BIT);
		inline SubpassAttachmentHandle
		AddColorAttachment(const char *attachment_str, VkImageLayout layout, const char *generator_subpass_str,
		                   VkPipelineStageFlags generator_stage, VkPipelineStageFlags use_stage,
		                   VkAccessFlags generator_access, VkAccessFlags use_access,
		                   VkDependencyFlags dependency_flag = VK_DEPENDENCY_BY_REGION_BIT) {
			return AddColorResolveAttachment(attachment_str, layout, nullptr, VK_IMAGE_LAYOUT_UNDEFINED,
			                                 generator_subpass_str, generator_stage, use_stage, generator_access,
			                                 use_access, dependency_flag);
		}
		inline SubpassAttachmentHandle
		AddDefaultColorAttachment(const char *attachment_str, const char *generator_subpass_str,
		                          VkDependencyFlags dependency_flag = VK_DEPENDENCY_BY_REGION_BIT) {
			return AddColorAttachment(
			    attachment_str, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, generator_subpass_str,
			    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, dependency_flag);
		}
		SubpassAttachmentHandle AddInputAttachment(const char *attachment_str, VkImageLayout layout,
		                                           const char *generator_subpass_str,
		                                           VkPipelineStageFlags generator_stage, VkPipelineStageFlags use_stage,
		                                           VkAccessFlags generator_access, VkAccessFlags use_access,
		                                           VkDependencyFlags dependency_flag = VK_DEPENDENCY_BY_REGION_BIT);
		inline SubpassAttachmentHandle
		AddDefaultInputAttachment(const char *attachment_str, const char *generator_subpass_str,
		                          VkPipelineStageFlags generator_stage, VkPipelineStageFlags use_stage,
		                          VkAccessFlags generator_access, VkAccessFlags use_access,
		                          VkDependencyFlags dependency_flag = VK_DEPENDENCY_BY_REGION_BIT) {
			return AddInputAttachment(attachment_str, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, generator_subpass_str,
			                          generator_stage, use_stage, generator_access, use_access, dependency_flag);
		}
		inline SubpassAttachmentHandle
		AddDefaultColorInputAttachment(const char *attachment_str, const char *generator_subpass_str,
		                               VkDependencyFlags dependency_flag = VK_DEPENDENCY_BY_REGION_BIT) {
			return AddInputAttachment(attachment_str, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, generator_subpass_str,
			                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			                          VK_ACCESS_SHADER_READ_BIT, dependency_flag);
		}
		/* SubpassAttachmentHandle AddPreserveAttachment(const char *attachment_str, const char *generator_subpass_str,
		                                              VkPipelineStageFlags generator_stage,
		                                              VkPipelineStageFlags use_stage, VkAccessFlags generator_access,
		                                              VkAccessFlags use_access,
		                                              VkDependencyFlags dependency_flag = VK_DEPENDENCY_BY_REGION_BIT);
		 */
		SubpassAttachmentHandle
		SetDepthStencilAttachment(const char *attachment_str, VkImageLayout layout, const char *src_subpass_str,
		                          VkPipelineStageFlags src_stage, VkPipelineStageFlags use_stage,
		                          VkAccessFlags src_access, VkAccessFlags use_access,
		                          VkDependencyFlags dependency_flag = VK_DEPENDENCY_BY_REGION_BIT);
		inline SubpassAttachmentHandle
		SetReadOnlyDepthStencilAttachment(const char *attachment_str, const char *src_subpass_str,
		                                  VkDependencyFlags dependency_flag = VK_DEPENDENCY_BY_REGION_BIT) {
			return SetDepthStencilAttachment(
			    attachment_str, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, src_subpass_str,
			    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT, dependency_flag);
		}
		inline SubpassAttachmentHandle
		SetReadWriteDepthStencilAttachment(const char *attachment_str, const char *src_subpass_str,
		                                   VkDependencyFlags dependency_flag = VK_DEPENDENCY_BY_REGION_BIT) {
			return SetDepthStencilAttachment(
			    attachment_str, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, src_subpass_str,
			    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
			    dependency_flag);
		}
	};

private:
	std::map<std::string, uint32_t> m_subpass_map, m_attachment_map;
	uint32_t get_subpass(const char *subpass_str) const;
	uint32_t get_attachment(const char *attachment_str) const;
	std::vector<VkAttachmentDescription> m_attachment_descriptions;
	struct SubpassInfo {
		std::vector<VkAttachmentReference> color_attachment_references, input_attachment_references,
		    resolve_attachment_references;
		// TODO: preserve attachments?
		// std::vector<uint32_t> preserve_attachments;
		VkAttachmentReference depth_stencil_attachment_reference;
	};
	std::vector<SubpassInfo> m_subpass_infos;
	std::vector<VkSubpassDescription> m_subpass_descriptions;

	inline uint32_t get_subpass_count() const { return m_subpass_infos.size(); }
	inline uint32_t get_attachment_count() const { return m_attachment_descriptions.size(); }

	void insert_subpass_dependency(uint32_t src_subpass, uint32_t dst_subpass, VkPipelineStageFlags src_stage,
	                               VkPipelineStageFlags dst_stage, VkAccessFlags src_access, VkAccessFlags dst_access,
	                               VkDependencyFlags dependency_flag = VK_DEPENDENCY_BY_REGION_BIT);
	struct SubpassDependencyKey {
		uint32_t subpass_from, subpass_to;
		VkDependencyFlags dependency_flag;
		inline bool operator==(const SubpassDependencyKey &r) const {
			return subpass_from == r.subpass_from && subpass_to == r.subpass_to && dependency_flag == r.dependency_flag;
		}
		inline bool operator<(const SubpassDependencyKey &r) const {
			return std::tie(subpass_from, subpass_to, dependency_flag) <
			       std::tie(r.subpass_from, r.subpass_to, r.dependency_flag);
		}
	};
	std::map<SubpassDependencyKey, uint32_t> m_subpass_dependency_indices;
	std::vector<VkSubpassDependency> m_subpass_dependencies;

public:
	void Initialize(uint32_t subpass_count, uint32_t attachment_count);

	RenderPassState() = default;
	inline RenderPassState(uint32_t subpass_count, uint32_t attachment_count) {
		Initialize(subpass_count, attachment_count);
	}

	SubpassAttachmentHandle RegisterSubpass(uint32_t subpass, const char *subpass_str,
	                                        VkSubpassDescriptionFlags flags = 0);
	void RegisterAttachment(uint32_t attachment, const char *attachment_str, VkFormat format,
	                        VkImageLayout initial_layout, VkImageLayout final_layout, VkSampleCountFlagBits samples,
	                        VkAttachmentLoadOp load_op, VkAttachmentStoreOp store_op,
	                        VkAttachmentLoadOp stencil_load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	                        VkAttachmentStoreOp stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE);

	void AddExtraSubpassDependency(const char *src_subpass_str, const char *dst_subpass_str,
	                               VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
	                               VkAccessFlags src_access, VkAccessFlags dst_access,
	                               VkDependencyFlags dependency_flag);

	void PopRenderPassCreateInfo(VkRenderPassCreateInfo *info) const;
};

} // namespace myvk

#endif

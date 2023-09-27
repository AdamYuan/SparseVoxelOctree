#ifndef MYVK_RG_RESOURCE_USAGE_HPP
#define MYVK_RG_RESOURCE_USAGE_HPP

#include "ResourceBase.hpp"

namespace myvk_rg {
enum class Usage {
	kColorAttachmentW,
	kColorAttachmentRW,
	kDepthAttachmentR,
	kDepthAttachmentRW,
	kInputAttachment,
	kSampledImage,
	kStorageImageR,
	kStorageImageW,
	kStorageImageRW,
	kUniformBuffer,
	kStorageBufferR,
	kStorageBufferW,
	kStorageBufferRW,
	kIndexBuffer,
	kVertexBuffer,
	kDrawIndirectBuffer,
	kTransferImageSrc,
	kTransferImageDst,
	kTransferBufferSrc,
	kTransferBufferDst,
	___USAGE_NUM
};
}

namespace myvk_rg::_details_ {

struct UsageInfo {
	VkAccessFlags2 read_access_flags, write_access_flags;

	ResourceType resource_type;
	VkFlags resource_creation_usages;
	VkImageLayout image_layout;

	VkPipelineStageFlags2 specified_pipeline_stages;
	VkPipelineStageFlags2 optional_pipeline_stages;

	bool is_descriptor;
	VkDescriptorType descriptor_type;
};
inline constexpr VkPipelineStageFlags2 __PIPELINE_STAGE_ALL_SHADERS_BIT =
    VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT |
    VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT |
    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

template <Usage> inline constexpr UsageInfo kUsageInfo{};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kColorAttachmentW> = {0,
                                                                   VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                                                   ResourceType::kImage,
                                                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                   VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                                   0,
                                                                   false,
                                                                   {}};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kColorAttachmentRW> = {VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                                                                    VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                                                    ResourceType::kImage,
                                                                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                                    0,
                                                                    false,
                                                                    {}};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kDepthAttachmentR> = {VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                                                   0,
                                                                   ResourceType::kImage,
                                                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                                   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                                   VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
                                                                   0,
                                                                   false,
                                                                   {}};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kDepthAttachmentRW> = {VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                                                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                                                    ResourceType::kImage,
                                                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                                    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                                                                        VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                                                                    0,
                                                                    false,
                                                                    {}};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kInputAttachment> = {VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT,
                                                                  0,
                                                                  ResourceType::kImage,
                                                                  VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                                  VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                                                  0,
                                                                  true,
                                                                  VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kSampledImage> = {VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                                                               0,
                                                               ResourceType::kImage,
                                                               VK_IMAGE_USAGE_SAMPLED_BIT,
                                                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                               0,
                                                               __PIPELINE_STAGE_ALL_SHADERS_BIT,
                                                               true,
                                                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kStorageImageR> = {VK_ACCESS_2_SHADER_STORAGE_READ_BIT, //
                                                                0,
                                                                ResourceType::kImage,
                                                                VK_IMAGE_USAGE_STORAGE_BIT,
                                                                VK_IMAGE_LAYOUT_GENERAL,
                                                                0,
                                                                __PIPELINE_STAGE_ALL_SHADERS_BIT,
                                                                true,
                                                                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kStorageImageW> = {0,
                                                                VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
                                                                ResourceType::kImage,
                                                                VK_IMAGE_USAGE_STORAGE_BIT,
                                                                VK_IMAGE_LAYOUT_GENERAL,
                                                                0,
                                                                __PIPELINE_STAGE_ALL_SHADERS_BIT,
                                                                true,
                                                                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kStorageImageRW> = {VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
                                                                 VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
                                                                 ResourceType::kImage,
                                                                 VK_IMAGE_USAGE_STORAGE_BIT,
                                                                 VK_IMAGE_LAYOUT_GENERAL,
                                                                 0,
                                                                 __PIPELINE_STAGE_ALL_SHADERS_BIT,
                                                                 true,
                                                                 VK_DESCRIPTOR_TYPE_STORAGE_IMAGE};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kUniformBuffer> = {VK_ACCESS_2_UNIFORM_READ_BIT, //
                                                                0,
                                                                ResourceType::kBuffer,
                                                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                                {},
                                                                0,
                                                                __PIPELINE_STAGE_ALL_SHADERS_BIT,
                                                                true,
                                                                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kStorageBufferR> = {VK_ACCESS_2_SHADER_STORAGE_READ_BIT, //
                                                                 0,
                                                                 ResourceType::kBuffer,
                                                                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                                 {},
                                                                 0,
                                                                 __PIPELINE_STAGE_ALL_SHADERS_BIT,
                                                                 true,
                                                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kStorageBufferW> = {0, //
                                                                 VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
                                                                 ResourceType::kBuffer,
                                                                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                                 {},
                                                                 0,
                                                                 __PIPELINE_STAGE_ALL_SHADERS_BIT,
                                                                 true,
                                                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kStorageBufferRW> = {VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
                                                                  VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
                                                                  ResourceType::kBuffer,
                                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                                  {},
                                                                  0,
                                                                  __PIPELINE_STAGE_ALL_SHADERS_BIT,
                                                                  true,
                                                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kVertexBuffer> = {VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT,
                                                               0,
                                                               ResourceType::kBuffer,
                                                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                               {},
                                                               VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT,
                                                               0,
                                                               false,
                                                               {}};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kIndexBuffer> = {VK_ACCESS_2_INDEX_READ_BIT, //
                                                              0,
                                                              ResourceType::kBuffer,
                                                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                              {},
                                                              VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT,
                                                              0,
                                                              false,
                                                              {}};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kDrawIndirectBuffer> = {VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT, //
                                                                     0,
                                                                     ResourceType::kBuffer,
                                                                     VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                                     {},
                                                                     VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT,
                                                                     0,
                                                                     false,
                                                                     {}};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kTransferImageSrc> = {
    VK_ACCESS_2_TRANSFER_READ_BIT, //
    0,
    ResourceType::kImage,
    VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    0,
    VK_PIPELINE_STAGE_2_COPY_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT, // Copy or Blit as SRC
    false,
    {}};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kTransferImageDst> = {
    0, //
    VK_ACCESS_2_TRANSFER_WRITE_BIT,
    ResourceType::kImage,
    VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    0,
    VK_PIPELINE_STAGE_2_COPY_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT |
        VK_PIPELINE_STAGE_2_CLEAR_BIT, // Copy, Blit, Clear as DST
    false,
    {}};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kTransferBufferSrc> = {VK_ACCESS_2_TRANSFER_READ_BIT, //
                                                                    0,
                                                                    ResourceType::kBuffer,
                                                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                    {},
                                                                    VK_PIPELINE_STAGE_2_COPY_BIT, // ONLY Copy as SRC
                                                                    0,
                                                                    false,
                                                                    {}};
template <>
inline constexpr UsageInfo kUsageInfo<Usage::kTransferBufferDst> = {
    0, //
    VK_ACCESS_2_TRANSFER_WRITE_BIT,
    ResourceType::kBuffer,
    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    {},
    0,
    VK_PIPELINE_STAGE_2_COPY_BIT | VK_PIPELINE_STAGE_2_CLEAR_BIT, // Copy or Fill as DST
    false,
    {}};
template <typename IndexSequence> class UsageInfoTable;
template <std::size_t... Indices> class UsageInfoTable<std::index_sequence<Indices...>> {
private:
	inline static constexpr UsageInfo kArr[] = {(kUsageInfo<(Usage)Indices>)...};

public:
	inline constexpr const UsageInfo &operator[](Usage usage) const { return kArr[(std::size_t)usage]; }
};
inline constexpr UsageInfoTable<std::make_index_sequence<(std::size_t)Usage::___USAGE_NUM>> kUsageInfoTable{};

// Is Descriptor
template <Usage Usage> inline constexpr bool kUsageIsDescriptor = kUsageInfo<Usage>.is_descriptor;
inline constexpr bool UsageIsDescriptor(Usage usage) { return kUsageInfoTable[usage].is_descriptor; }
// Get Descriptor Type
template <Usage Usage> inline constexpr VkDescriptorType kUsageGetDescriptorType = kUsageInfo<Usage>.descriptor_type;
inline constexpr VkDescriptorType UsageGetDescriptorType(Usage usage) { return kUsageInfoTable[usage].descriptor_type; }
// Get Read Access Flags
template <Usage Usage> inline constexpr VkAccessFlags2 kUsageGetReadAccessFlags = kUsageInfo<Usage>.read_access_flags;
inline constexpr VkAccessFlags2 UsageGetReadAccessFlags(Usage usage) {
	return kUsageInfoTable[usage].read_access_flags;
}
// Get Write Access Flags
template <Usage Usage> inline constexpr VkAccessFlags2 kUsageGetWriteAccessFlags = kUsageInfo<Usage>.write_access_flags;
inline constexpr VkAccessFlags2 UsageGetWriteAccessFlags(Usage usage) {
	return kUsageInfoTable[usage].write_access_flags;
}
// Get Access Flags
template <Usage Usage>
inline constexpr VkAccessFlags2 kUsageGetAccessFlags =
    kUsageGetReadAccessFlags<Usage> | kUsageGetWriteAccessFlags<Usage>;
inline constexpr VkAccessFlags2 UsageGetAccessFlags(Usage usage) {
	return UsageGetReadAccessFlags(usage) | UsageGetWriteAccessFlags(usage);
}
// Is Read-Only
template <Usage Usage> inline constexpr bool kUsageIsReadOnly = kUsageGetWriteAccessFlags<Usage> == 0;
inline constexpr bool UsageIsReadOnly(Usage usage) { return UsageGetWriteAccessFlags(usage) == 0; }
// Is For Buffer
template <Usage Usage> inline constexpr bool kUsageForBuffer = kUsageInfo<Usage>.resource_type == ResourceType::kBuffer;
inline constexpr bool UsageForBuffer(Usage usage) {
	return kUsageInfoTable[usage].resource_type == ResourceType::kBuffer;
}
// Is For Image
template <Usage Usage> inline constexpr bool kUsageForImage = kUsageInfo<Usage>.resource_type == ResourceType::kImage;
inline constexpr bool UsageForImage(Usage usage) {
	return kUsageInfoTable[usage].resource_type == ResourceType::kImage;
}
// Is Color Attachment
template <Usage Usage>
inline constexpr bool kUsageIsColorAttachment = Usage == Usage::kColorAttachmentW || Usage == Usage::kColorAttachmentRW;
inline constexpr bool UsageIsColorAttachment(Usage usage) {
	return usage == Usage::kColorAttachmentW || usage == Usage::kColorAttachmentRW;
}
// Is Depth Attachment
template <Usage Usage>
inline constexpr bool kUsageIsDepthAttachment = Usage == Usage::kDepthAttachmentR || Usage == Usage::kDepthAttachmentRW;
inline constexpr bool UsageIsDepthAttachment(Usage usage) {
	return usage == Usage::kDepthAttachmentR || usage == Usage::kDepthAttachmentRW;
}
// Is Attachment
template <Usage Usage>
inline constexpr bool kUsageIsAttachment =
    kUsageIsColorAttachment<Usage> || kUsageIsDepthAttachment<Usage> || Usage == Usage::kInputAttachment;
inline constexpr bool UsageIsAttachment(Usage usage) {
	return UsageIsColorAttachment(usage) || UsageIsDepthAttachment(usage) || usage == Usage::kInputAttachment;
}
// Has Specified Pipeline Stages
template <Usage Usage>
inline constexpr bool kUsageHasSpecifiedPipelineStages = kUsageInfo<Usage>.specified_pipeline_stages;
inline constexpr bool UsageHasSpecifiedPipelineStages(Usage usage) {
	return kUsageInfoTable[usage].specified_pipeline_stages;
}
// Get Specified Pipeline Stages
template <Usage Usage>
inline constexpr VkPipelineStageFlags2 kUsageGetSpecifiedPipelineStages = kUsageInfo<Usage>.specified_pipeline_stages;
inline constexpr VkPipelineStageFlags2 UsageGetSpecifiedPipelineStages(Usage usage) {
	return kUsageInfoTable[usage].specified_pipeline_stages;
}
// Get Optional Pipeline Stages
template <Usage Usage>
inline constexpr VkPipelineStageFlags2 kUsageGetOptionalPipelineStages = kUsageInfo<Usage>.optional_pipeline_stages;
inline constexpr VkPipelineStageFlags2 UsageGetOptionalPipelineStages(Usage usage) {
	return kUsageInfoTable[usage].optional_pipeline_stages;
}
// Get Image Layout
template <Usage Usage> inline constexpr VkImageLayout kUsageGetImageLayout = kUsageInfo<Usage>.image_layout;
inline constexpr VkImageLayout UsageGetImageLayout(Usage usage) { return kUsageInfoTable[usage].image_layout; }
// Get Resource Creation Usages
template <Usage Usage> inline constexpr VkFlags kUsageGetCreationUsages = kUsageInfo<Usage>.resource_creation_usages;
inline constexpr VkFlags UsageGetCreationUsages(Usage usage) { return kUsageInfoTable[usage].resource_creation_usages; }
// inline constexpr bool UsageAll(Usage) { return true; }

// Input Usage Operation
/*using UsageClass = bool(Usage);
namespace _details_rg_input_usage_op_ {
template <UsageClass... Args> struct Union;
template <UsageClass X, UsageClass... Args> struct Union<X, Args...> {
    constexpr bool operator()(Usage x) const { return X(x) || Union<Args...>(x); }
};
template <> struct Union<> {
    constexpr bool operator()(Usage) const { return false; }
};
template <bool... Args(Usage)> struct Intersect;
template <UsageClass X, UsageClass... Args> struct Intersect<X, Args...> {
    constexpr bool operator()(Usage x) const { return X(x) && Intersect<Args...>(x); }
};
template <> struct Intersect<> {
    constexpr bool operator()(Usage) const { return true; }
};
} // namespace _details_rg_input_usage_op_
// Union: A | B
template <UsageClass... Args> inline constexpr bool UsageUnion(Usage x) {
    return _details_rg_input_usage_op_::Union<Args...>(x);
}
// Intersect: A & B
template <UsageClass... Args> inline constexpr bool UsageIntersect(Usage x) {
    return _details_rg_input_usage_op_::Intersect<Args...>(x);
}
// Complement: ~X
template <UsageClass X> inline constexpr bool UsageComplement(Usage x) { return !X(x); }
// Minus: A & (~B)
template <UsageClass A, UsageClass B> inline constexpr bool UsageMinus(Usage x) {
    return UsageIntersect<A, UsageComplement<B>>(x);
}*/

} // namespace myvk_rg::_details_

#endif

#ifndef MYVK_RG_RENDER_GRAPH_RESOLVER_HPP
#define MYVK_RG_RENDER_GRAPH_RESOLVER_HPP

#include <cinttypes>
#include <vector>

#include "Bitset.hpp"

#include <myvk_rg/_details_/Pass.hpp>
#include <myvk_rg/_details_/RenderGraphBase.hpp>
#include <myvk_rg/_details_/Resource.hpp>

namespace myvk_rg::_details_ {

enum class DependencyType : uint8_t { kDependency, kValidation, kExternal, kLastFrame };

struct ResourceReference {
	const Input *p_input;
	const PassBase *pass;
};

class RenderGraphResolver {
public:
	struct LFResourceInfo {
		const ResourceBase *lf_resource;
		std::vector<ResourceReference> references, last_references;
	};
	struct IntResourceInfo {
		LFResourceInfo *p_last_frame_info;
		std::vector<ResourceReference> references, validation_references, last_references;
	};
	struct IntBufferInfo : public IntResourceInfo {
		const ManagedBuffer *buffer{};
	};
	struct IntImageInfo : public IntResourceInfo {
		const InternalImageBase *image{};
		bool is_transient{};
	};
	struct IntImageViewInfo {
		const InternalImageBase *image{};
	};

	struct PassEdge {
		const ResourceBase *resource{};
		ResourceReference from{}, to{};
		DependencyType type{};
	};
	struct PassNode {
		const PassBase *pass;
		std::vector<const PassEdge *> input_edges, output_edges;
	};

private:
	struct OriginGraph; // The Graph Containing Passes and Internal Resources

	std::vector<LFResourceInfo> m_last_frame_resources;

	std::vector<IntImageInfo> m_internal_images;
	std::vector<IntImageViewInfo> m_internal_image_views;
	std::vector<IntBufferInfo> m_internal_buffers;

	RelationMatrix /*m_image_view_contain_relation, */ m_resource_conflict_relation, m_resource_not_prior_relation,
	    m_pass_prior_relation, m_pass_prior_relation_transpose;

	std::vector<PassNode> m_pass_nodes;
	std::vector<PassEdge> m_pass_edges;
	std::vector<const PassEdge *> m_src_output_edges;

	static OriginGraph make_origin_graph(const RenderGraphBase *p_render_graph);
	static void _initialize_combined_image(const CombinedImage *image);
	void extract_resources(const OriginGraph &graph);
	void extract_ordered_passes_and_edges(OriginGraph &&graph);

	void extract_pass_relation();
	void extract_resource_relation();
	void extract_resource_references();

public:
	void Resolve(const RenderGraphBase *p_render_graph);

	inline uint32_t GetPassNodeCount() const { return m_pass_nodes.size(); }
	inline const PassNode &GetPassNode(uint32_t pass_order) const { return m_pass_nodes[pass_order]; }
	inline const PassNode &GetPassNode(const PassBase *pass) const { return m_pass_nodes[GetPassOrder(pass)]; }
	inline static uint32_t GetPassOrder(const PassBase *pass) { return pass->m_resolved_info.pass_order; }
	inline const std::vector<PassNode> &GetPassNodes() const { return m_pass_nodes; }
	inline const std::vector<PassEdge> &GetPassEdges() const { return m_pass_edges; }
	inline const std::vector<const PassEdge *> &GetSrcOutputEdges() const { return m_src_output_edges; }

	inline uint32_t GetIntBufferCount() const { return m_internal_buffers.size(); }
	inline const IntBufferInfo &GetIntBufferInfo(uint32_t buffer_id) const { return m_internal_buffers[buffer_id]; }
	inline IntBufferInfo &GetIntBufferInfo(uint32_t buffer_id) { return m_internal_buffers[buffer_id]; }
	template <typename Buffer> inline const IntBufferInfo &GetIntBufferInfo(const Buffer *buffer) const {
		return m_internal_buffers[GetIntBufferID(buffer)];
	}
	template <typename Buffer> inline IntBufferInfo &GetIntBufferInfo(const Buffer *buffer) {
		return m_internal_buffers[GetIntBufferID(buffer)];
	}
	inline const std::vector<IntBufferInfo> &GetIntBufferInfos() const { return m_internal_buffers; }

	inline uint32_t GetIntImageCount() const { return m_internal_images.size(); }
	inline const IntImageInfo &GetIntImageInfo(uint32_t image_id) const { return m_internal_images[image_id]; }
	inline IntImageInfo &GetIntImageInfo(uint32_t image_id) { return m_internal_images[image_id]; }
	template <typename Image> inline const IntImageInfo &GetIntImageInfo(const Image *image) const {
		return m_internal_images[GetIntImageID(image)];
	}
	template <typename Image> inline IntImageInfo &GetIntImageInfo(const Image *image) {
		return m_internal_images[GetIntImageID(image)];
	}
	inline const std::vector<IntImageInfo> &GetIntImageInfos() const { return m_internal_images; }

	inline uint32_t GetIntResourceCount() const { return GetIntBufferCount() + GetIntImageCount(); }
	inline IntResourceInfo &GetIntResourceInfo(uint32_t resource_id) {
		return resource_id < GetIntImageCount() ? (IntResourceInfo &)GetIntImageInfo(resource_id)
		                                        : (IntResourceInfo &)GetIntBufferInfo(resource_id - GetIntImageCount());
	}
	inline const IntResourceInfo &GetIntResourceInfo(uint32_t resource_id) const {
		return resource_id < GetIntImageCount()
		           ? (const IntResourceInfo &)GetIntImageInfo(resource_id)
		           : (const IntResourceInfo &)GetIntBufferInfo(resource_id - GetIntImageCount());
	}
	template <typename Resource> inline const IntResourceInfo &GetIntResourceInfo(const Resource *resource) const {
		return GetIntResourceInfo(GetIntResourceID(resource));
	}
	template <typename Resource> inline IntResourceInfo &GetIntResourceInfo(const Resource *resource) {
		return GetIntResourceInfo(GetIntResourceID(resource));
	}

	inline uint32_t GetIntImageViewCount() const { return m_internal_image_views.size(); }
	inline const IntImageViewInfo &GetIntImageViewInfo(uint32_t image_view_id) const {
		return m_internal_image_views[image_view_id];
	}
	inline IntImageViewInfo &GetIntImageViewInfo(uint32_t image_view_id) {
		return m_internal_image_views[image_view_id];
	}
	template <typename Image> inline const IntImageViewInfo &GetIntImageViewInfo(const Image *image) const {
		return m_internal_image_views[GetIntImageViewID(image)];
	}
	template <typename Image> inline IntImageViewInfo &GetIntImageViewInfo(const Image *image) {
		return m_internal_image_views[GetIntImageViewID(image)];
	}
	inline const std::vector<IntImageViewInfo> &GetIntImageViewInfos() const { return m_internal_image_views; }

	// Get Internal Buffer ID
	inline static uint32_t GetIntBufferID(const ManagedBuffer *buffer) { return buffer->m_resolved_info.buffer_id; }
	inline static uint32_t GetIntBufferID(const ExternalBufferBase *) { return -1; }
	inline static uint32_t GetIntBufferID(const BufferAlias *buffer) {
		return buffer->GetPointedResource()->Visit(
		    [](const auto *buffer) -> uint32_t { return GetIntBufferID(buffer); });
	}
	inline static uint32_t GetIntBufferID(const LastFrameBuffer *buffer) {
		return GetIntBufferID(buffer->GetCurrentResource());
	}
	inline static uint32_t GetIntBufferID(const BufferBase *buffer) {
		return buffer->Visit([](const auto *buffer) -> uint32_t { return GetIntBufferID(buffer); });
	}

	// Get Internal ImageView ID
	inline static uint32_t GetIntImageViewID(const InternalImageBase *image) {
		return image->m_resolved_info.image_view_id;
	}
	inline static uint32_t GetIntImageViewID(const ExternalImageBase *) { return -1; }
	inline static uint32_t GetIntImageViewID(const ImageAlias *image) {
		return image->GetPointedResource()->Visit(
		    [](const auto *image) -> uint32_t { return GetIntImageViewID(image); });
	}
	inline static uint32_t GetIntImageViewID(const LastFrameImage *image) {
		return GetIntImageViewID(image->GetCurrentResource());
	}
	inline static uint32_t GetIntImageViewID(const ImageBase *image) {
		return image->Visit([](const auto *image) -> uint32_t { return GetIntImageViewID(image); });
	}

	// Get Internal Image ID
	inline static uint32_t GetIntImageID(const InternalImageBase *image) { return image->m_resolved_info.image_id; }
	inline static uint32_t GetIntImageID(const ExternalImageBase *image) { return -1; }
	inline static uint32_t GetIntImageID(const LastFrameImage *image) {
		return GetIntImageID(image->GetCurrentResource());
	}
	inline static uint32_t GetIntImageID(const ImageAlias *image) {
		return image->GetPointedResource()->Visit([](const auto *image) -> uint32_t { return GetIntImageID(image); });
	}
	inline static uint32_t GetIntImageID(const ImageBase *image) {
		return image->Visit([](const auto *image) -> uint32_t { return GetIntImageID(image); });
	}

	// Get Internal Resource ID
	inline uint32_t GetIntResourceID(const ManagedBuffer *buffer) const {
		return GetIntBufferID(buffer) + GetIntImageCount();
	}
	inline static uint32_t GetIntResourceID(const ExternalBufferBase *) { return -1; }
	inline uint32_t GetIntResourceID(const BufferAlias *buffer) const {
		return buffer->GetPointedResource()->Visit(
		    [this](const auto *buffer) -> uint32_t { return GetIntResourceID(buffer); });
	}
	inline uint32_t GetIntResourceID(const LastFrameBuffer *buffer) {
		return GetIntResourceID(buffer->GetCurrentResource());
	}
	inline uint32_t GetIntResourceID(const BufferBase *buffer) const {
		return buffer->Visit([this](const auto *buffer) -> uint32_t { return GetIntResourceID(buffer); });
	}
	inline static uint32_t GetIntResourceID(const InternalImageBase *image) { return GetIntImageID(image); }
	inline static uint32_t GetIntResourceID(const ExternalImageBase *) { return -1; }
	inline static uint32_t GetIntResourceID(const LastFrameImage *image) { return GetIntImageID(image); }
	inline static uint32_t GetIntResourceID(const ImageBase *image) { return GetIntImageID(image); }

	inline uint32_t GetIntResourceID(const ResourceBase *resource) const {
		return resource->Visit([this](const auto *resource) -> uint32_t { return GetIntResourceID(resource); });
	}

	/* inline bool IsParentIntImageView(uint32_t parent_image_view, uint32_t cur_image_view) const {
	    return m_image_view_parent_relation.GetRelation(parent_image_view, cur_image_view);
	} */
	inline bool IsIntResourceConflicted(uint32_t resource_0, uint32_t resource_1) const {
		return m_resource_conflict_relation.GetRelation(resource_0, resource_1);
	}
	template <typename Resource0, typename Resource1>
	inline bool IsIntResourceConflicted(const Resource0 *resource_0, const Resource1 *resource_1) const {
		return m_resource_conflict_relation.GetRelation(GetIntResourceID(resource_0), GetIntResourceID(resource_1));
	}
	inline bool IsIntResourcePrior(uint32_t resource_0, uint32_t resource_1) const {
		return !m_resource_not_prior_relation.GetRelation(resource_0, resource_1);
	}
	template <typename Resource0, typename Resource1>
	inline bool IsIntResourcePrior(const Resource0 *resource_0, const Resource1 *resource_1) const {
		return !m_resource_not_prior_relation.GetRelation(GetIntResourceID(resource_0), GetIntResourceID(resource_1));
	}
	inline bool IsPassPrior(uint32_t pass_order_0, uint32_t pass_order_1) const {
		return m_pass_prior_relation.GetRelation(pass_order_0, pass_order_1);
	}
	inline bool IsPassPrior(const PassBase *pass_0, const PassBase *pass_1) const {
		return m_pass_prior_relation.GetRelation(GetPassOrder(pass_0), GetPassOrder(pass_1));
	}

	/*inline bool IsIntImageViewContain(uint32_t image_view_0, uint32_t image_view_1) const {
	    return m_image_view_contain_relation.GetRelation(image_view_0, image_view_1);
	}
	template <typename ImageView0, typename ImageView1>
	inline bool IsIntImageViewContain(const ImageView0 *image_view_0, const ImageView1 *image_view_1) const {
	    return m_image_view_contain_relation.GetRelation(GetIntImageViewID(image_view_0),
	                                                     GetIntImageViewID(image_view_1));
	}*/
};

} // namespace myvk_rg::_details_

#endif

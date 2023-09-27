#ifndef MYVK_RG_RESOURCE_HPP
#define MYVK_RG_RESOURCE_HPP

#include <myvk/Buffer.hpp>
#include <myvk/FrameManager.hpp>
#include <myvk/ImageView.hpp>

#include "Macro.hpp"
#include "Pool.hpp"
#include "RenderGraphBase.hpp"
#include "ResourceBase.hpp"

#include <cassert>
#include <cinttypes>
#include <type_traits>

namespace myvk_rg {
enum class BufferMapType : uint8_t { kNone, kRandom, kSeqWrite };
}

namespace myvk_rg::_details_ {
class BufferBase : public ResourceBase {
public:
	inline constexpr ResourceType GetType() const { return ResourceType::kBuffer; }

	inline ~BufferBase() override = default;
	inline explicit BufferBase(ResourceState state) : ResourceBase(MakeResourceClass(ResourceType::kBuffer, state)) {}
	inline BufferBase(BufferBase &&) noexcept = default;

	template <typename Visitor> std::invoke_result_t<Visitor, ManagedBuffer *> inline Visit(Visitor &&visitor);
	template <typename Visitor> std::invoke_result_t<Visitor, ManagedBuffer *> inline Visit(Visitor &&visitor) const;

	inline const myvk::Ptr<myvk::BufferBase> &GetVkBuffer() const {
		return Visit([](auto *buffer) -> const myvk::Ptr<myvk::BufferBase> & { return buffer->GetVkBuffer(); });
	};
};

class ImageBase : public ResourceBase {
public:
	inline constexpr ResourceType GetType() const { return ResourceType::kImage; }

	inline ~ImageBase() override = default;
	inline explicit ImageBase(ResourceState state) : ResourceBase(MakeResourceClass(ResourceType::kImage, state)) {}
	inline ImageBase(ImageBase &&) noexcept = default;

	template <typename Visitor> std::invoke_result_t<Visitor, ManagedImage *> inline Visit(Visitor &&visitor);
	template <typename Visitor> std::invoke_result_t<Visitor, ManagedImage *> inline Visit(Visitor &&visitor) const;

	inline const myvk::Ptr<myvk::ImageView> &GetVkImageView() const {
		return Visit([](auto *image) -> const myvk::Ptr<myvk::ImageView> & { return image->GetVkImageView(); });
	};
	inline VkFormat GetFormat() const {
		return Visit([](auto *image) -> VkFormat { return image->GetFormat(); });
	}
};

template <typename Derived> class ImageAttachmentInfo {
private:
	inline RenderGraphBase *get_render_graph_ptr() {
		static_assert(std::is_base_of_v<ObjectBase, Derived>);
		return static_cast<ObjectBase *>(static_cast<Derived *>(this))->GetRenderGraphPtr();
	}
	VkAttachmentLoadOp m_load_op{VK_ATTACHMENT_LOAD_OP_DONT_CARE};
	VkClearValue m_clear_value{};

public:
	inline ImageAttachmentInfo() = default;
	inline ImageAttachmentInfo(VkAttachmentLoadOp load_op, const VkClearValue &clear_value)
	    : m_load_op{load_op}, m_clear_value{clear_value} {
		static_assert(std::is_base_of_v<ObjectBase, Derived>);
	}

	inline void SetLoadOp(VkAttachmentLoadOp load_op) {
		if (m_load_op != load_op) {
			m_load_op = load_op;
			get_render_graph_ptr()->SetCompilePhrases(CompilePhrase::kPrepareExecutor);
		}
	}
	inline void SetClearColorValue(const VkClearColorValue &clear_color_value) {
		m_clear_value.color = clear_color_value;
	}
	inline void SetClearDepthStencilValue(const VkClearDepthStencilValue &clear_depth_stencil_value) {
		m_clear_value.depthStencil = clear_depth_stencil_value;
	}

	inline VkAttachmentLoadOp GetLoadOp() const { return m_load_op; }
	inline const VkClearValue &GetClearValue() const { return m_clear_value; }
};

// External Resources
template <typename Derived> class ExternalResourceInfo {
private:
	inline RenderGraphBase *get_render_graph_ptr() {
		static_assert(std::is_base_of_v<ObjectBase, Derived>);
		return static_cast<ObjectBase *>(static_cast<Derived *>(this))->GetRenderGraphPtr();
	}
	inline RenderGraphBase *get_render_graph_ptr() const {
		static_assert(std::is_base_of_v<ObjectBase, Derived>);
		return static_cast<const ObjectBase *>(static_cast<const Derived *>(this))->GetRenderGraphPtr();
	}

	VkPipelineStageFlags2 m_src_stages{VK_PIPELINE_STAGE_2_NONE}, m_dst_stages{VK_PIPELINE_STAGE_2_NONE};
	VkAccessFlags2 m_src_accesses{VK_ACCESS_2_NONE}, m_dst_accesses{VK_ACCESS_2_NONE};

public:
	inline VkPipelineStageFlags2 GetSrcPipelineStages() const { return m_src_stages; }
	inline void SetSrcPipelineStages(VkPipelineStageFlags2 src_stages) {
		if (m_src_stages != src_stages) {
			m_src_stages = src_stages;
			get_render_graph_ptr()->SetCompilePhrase(CompilePhrase::kPrepareExecutor);
		}
	}
	inline VkPipelineStageFlags2 GetDstPipelineStages() const { return m_dst_stages; }
	inline void SetDstPipelineStages(VkPipelineStageFlags2 dst_stages) {
		if (m_dst_stages != dst_stages) {
			m_dst_stages = dst_stages;
			get_render_graph_ptr()->SetCompilePhrase(CompilePhrase::kPrepareExecutor);
		}
	}
	inline VkAccessFlags2 GetSrcAccessFlags() const { return m_src_accesses; }
	inline void SetSrcAccessFlags(VkAccessFlags2 src_accesses) {
		if (m_src_accesses != src_accesses) {
			m_src_accesses = src_accesses;
			get_render_graph_ptr()->SetCompilePhrase(CompilePhrase::kPrepareExecutor);
		}
	}
	inline VkAccessFlags2 GetDstAccessFlags() const { return m_dst_accesses; }
	inline void SetDstAccessFlags(VkAccessFlags2 dst_accesses) {
		if (m_dst_accesses != dst_accesses) {
			m_dst_accesses = dst_accesses;
			get_render_graph_ptr()->SetCompilePhrase(CompilePhrase::kPrepareExecutor);
		}
	}
	inline virtual bool IsStatic() const = 0;
};
class ExternalImageBase : public ImageBase,
                          public ImageAttachmentInfo<ExternalImageBase>,
                          public ExternalResourceInfo<ExternalImageBase> {
public:
	inline constexpr ResourceState GetState() const { return ResourceState::kExternal; }
	inline constexpr ResourceClass GetClass() const { return ResourceClass::kExternalImageBase; }

	virtual const myvk::Ptr<myvk::ImageView> &GetVkImageView() const = 0;
	inline VkFormat GetFormat() const { return GetVkImageView()->GetImagePtr()->GetFormat(); }

	inline ExternalImageBase() : ImageBase(ResourceState::kExternal) {}
	inline ExternalImageBase(ExternalImageBase &&) noexcept = default;
	inline ~ExternalImageBase() override = default;

private:
	VkImageLayout m_src_layout{VK_IMAGE_LAYOUT_UNDEFINED}, m_dst_layout{VK_IMAGE_LAYOUT_GENERAL};

public:
	inline VkImageLayout GetSrcLayout() const { return m_src_layout; }
	inline void SetSrcLayout(VkImageLayout src_layout) {
		if (m_src_layout != src_layout) {
			m_src_layout = src_layout;
			GetRenderGraphPtr()->SetCompilePhrases(CompilePhrase::kPrepareExecutor);
		}
	}
	inline VkImageLayout GetDstLayout() const { return m_dst_layout; }
	inline void SetDstLayout(VkImageLayout dst_layout) {
		if (m_dst_layout != dst_layout) {
			m_dst_layout = dst_layout;
			GetRenderGraphPtr()->SetCompilePhrases(CompilePhrase::kPrepareExecutor);
		}
	}
};

class ExternalBufferBase : public BufferBase, public ExternalResourceInfo<ExternalBufferBase> {
public:
	inline constexpr ResourceState GetState() const { return ResourceState::kExternal; }
	inline constexpr ResourceClass GetClass() const { return ResourceClass::kExternalBufferBase; }

	virtual const myvk::Ptr<myvk::BufferBase> &GetVkBuffer() const = 0;

	inline ExternalBufferBase() : BufferBase(ResourceState::kExternal) {}
	inline ExternalBufferBase(ExternalBufferBase &&) noexcept = default;
	inline ~ExternalBufferBase() override = default;
};
// Alias
class Input;
class ImageAlias final : public ImageBase {
private:
	const Input *m_producer_input{};
	const PassBase *m_producer_pass{};
	const ImageBase *m_pointed_image{};

	MYVK_RG_OBJECT_FRIENDS
	inline void Initialize(const PassBase *producer_pass, const Input *producer_input, const ImageBase *image) {
		m_producer_pass = producer_pass;
		m_producer_input = producer_input;
		m_pointed_image = image->Visit([](auto *image) -> const ImageBase * {
			if constexpr (ResourceVisitorTrait<decltype(image)>::kState == ResourceState::kAlias)
				return image->GetPointedResource();
			return image;
		});
	}

public:
	inline constexpr ResourceState GetState() const { return ResourceState::kAlias; }
	inline constexpr ResourceClass GetClass() const { return ResourceClass::kImageAlias; }

	inline ImageAlias() : ImageBase(ResourceState::kAlias) {}
	inline ImageAlias(ImageAlias &&) noexcept = default;
	inline ~ImageAlias() final = default;

	inline const ImageBase *GetPointedResource() const {
		return m_pointed_image->Visit([](auto *image) -> const ImageBase * {
			if constexpr (ResourceVisitorTrait<decltype(image)>::kState == ResourceState::kLastFrame)
				return image->GetCurrentResource();
			return image;
		});
	}
	inline const PassBase *GetProducerPass() const { return m_producer_pass; }
	inline const Input *GetProducerInput() const { return m_producer_input; }

	inline const myvk::Ptr<myvk::ImageView> &GetVkImageView() const { return m_pointed_image->GetVkImageView(); }
	inline VkFormat GetFormat() const { return m_pointed_image->GetFormat(); }
};
class BufferAlias final : public BufferBase {
private:
	const Input *m_producer_input{};
	const PassBase *m_producer_pass{};
	const BufferBase *m_pointed_buffer{};

	MYVK_RG_OBJECT_FRIENDS
	inline void Initialize(const PassBase *producer_pass, const Input *producer_input, const BufferBase *buffer) {
		m_producer_pass = producer_pass;
		m_producer_input = producer_input;
		m_pointed_buffer = buffer->Visit([](auto *buffer) -> const BufferBase * {
			if constexpr (ResourceVisitorTrait<decltype(buffer)>::kState == ResourceState::kAlias)
				return buffer->GetPointedResource();
			return buffer;
		});
	}

public:
	inline constexpr ResourceState GetState() const { return ResourceState::kAlias; }
	inline constexpr ResourceClass GetClass() const { return ResourceClass::kBufferAlias; }

	inline BufferAlias() : BufferBase(ResourceState::kAlias) {}
	inline BufferAlias(BufferAlias &&) = default;
	inline ~BufferAlias() final = default;

	inline const BufferBase *GetPointedResource() const {
		return m_pointed_buffer->Visit([](auto *buffer) -> const BufferBase * {
			if constexpr (ResourceVisitorTrait<decltype(buffer)>::kState == ResourceState::kLastFrame)
				return buffer->GetCurrentResource();
			return buffer;
		});
	}
	inline const PassBase *GetProducerPass() const { return m_producer_pass; }
	inline const Input *GetProducerInput() const { return m_producer_input; }

	inline const myvk::Ptr<myvk::BufferBase> &GetVkBuffer() const { return m_pointed_buffer->GetVkBuffer(); }
};

// Managed Resources
template <typename Derived, typename SizeType /*, typename VkUsageEnum*/> class ManagedResourceInfo {
public:
	using SizeFunc = std::function<SizeType(const VkExtent2D &)>;

private:
	inline RenderGraphBase *get_render_graph_ptr() {
		static_assert(std::is_base_of_v<ObjectBase, Derived>);
		return static_cast<ObjectBase *>(static_cast<Derived *>(this))->GetRenderGraphPtr();
	}
	inline RenderGraphBase *get_render_graph_ptr() const {
		static_assert(std::is_base_of_v<ObjectBase, Derived>);
		return static_cast<const ObjectBase *>(static_cast<const Derived *>(this))->GetRenderGraphPtr();
	}
	inline void set_size_changed_compile_phrease() const {
		get_render_graph_ptr()->SetCompilePhrases(CompilePhrase::kAllocate);
		if constexpr (std::is_base_of_v<ImageBase, Derived>) // Image Size change might affect RenderPass merging
			get_render_graph_ptr()->SetCompilePhrases(CompilePhrase::kSchedule);
	}

	mutable SizeType m_size{};
	SizeFunc m_size_func{};
	// VkUsageEnum m_extra_usages{};

public:
	inline const SizeType &GetSize() const {
		if (m_size_func)
			m_size = m_size_func(get_render_graph_ptr()->GetCanvasSize());
		return m_size;
	}
	template <typename... Args> inline void SetSize(Args &&...args) {
		SizeType size(std::forward<Args>(args)...);
		m_size_func = nullptr;
		if (m_size != size) {
			m_size = size;
			set_size_changed_compile_phrease();
		}
	}
	inline bool HaveSizeFunc() const { return m_size_func; }
	inline const SizeFunc &GetSizeFunc() const { return m_size_func; }
	inline void SetSizeFunc(const SizeFunc &func) {
		m_size_func = func;
		set_size_changed_compile_phrease();
	}
	/* inline void SetExtraUsages(VkUsageEnum extra_usages) {
	    m_extra_usages = extra_usages;
	    get_render_graph_ptr()->SetCompilePhrases(CompilePhrase::kAllocate);
	}
	inline VkUsageEnum GetExtraUsages() const { return m_extra_usages; } */
};

class ManagedBuffer final : public BufferBase, public ManagedResourceInfo<ManagedBuffer, VkDeviceSize> {
private:
	mutable struct {
	private:
		uint32_t buffer_id{};
		friend class RenderGraphResolver;
	} m_resolved_info{};

	BufferMapType m_map_type{BufferMapType::kNone};

	friend class RenderGraphResolver;

	MYVK_RG_OBJECT_FRIENDS
	inline void Initialize() {}

	void *get_mapped_data() const;

public:
	inline constexpr ResourceState GetState() const { return ResourceState::kManaged; }
	inline constexpr ResourceClass GetClass() const { return ResourceClass::kManagedBuffer; }

	inline BufferMapType GetMapType() const { return m_map_type; }
	inline void SetMapType(BufferMapType map_type) {
		if (m_map_type != map_type) {
			m_map_type = map_type;
			GetRenderGraphPtr()->SetCompilePhrases(CompilePhrase::kAllocate);
		}
	}
	void *GetMappedData() const;

	inline ManagedBuffer() : BufferBase(ResourceState::kManaged) {}
	inline ManagedBuffer(ManagedBuffer &&) noexcept = default;
	~ManagedBuffer() override = default;

	const myvk::Ptr<myvk::BufferBase> &GetVkBuffer() const;
	template <typename T = void> inline T *GetMappedData() const { return reinterpret_cast<T *>(get_mapped_data()); }
};

// Internal Image ( the combination of ManagedImage and CombinedImage, used in implementation )
class InternalImageBase : public ImageBase {
protected:
	mutable struct {
	private:
		uint32_t image_id{}, image_view_id{};
		friend class RenderGraphResolver;
	} m_resolved_info{};

	friend class RenderGraphResolver;

public:
	inline explicit InternalImageBase(ResourceState state) : ImageBase(state) {}

	template <typename Visitor> std::invoke_result_t<Visitor, ManagedImage *> inline Visit(Visitor &&visitor);
	template <typename Visitor> std::invoke_result_t<Visitor, ManagedImage *> inline Visit(Visitor &&visitor) const;

	inline const myvk::Ptr<myvk::ImageView> &GetVkImageView() const {
		return Visit([](auto *image) -> const myvk::Ptr<myvk::ImageView> & { return image->GetVkImageView(); });
	};
	inline VkFormat GetFormat() const {
		return Visit([](auto *image) -> VkFormat { return image->GetFormat(); });
	}
};

class SubImageSize {
private:
	VkExtent3D m_extent{};
	uint32_t m_layers{}, m_base_mip_level{}, m_mip_levels{};

public:
	inline SubImageSize() = default;
	inline explicit SubImageSize(const VkExtent3D &extent, uint32_t layers = 1, uint32_t base_mip_level = 0,
	                             uint32_t mip_levels = 1)
	    : m_extent{extent}, m_layers{layers}, m_base_mip_level{base_mip_level}, m_mip_levels{mip_levels} {}
	inline explicit SubImageSize(const VkExtent2D &extent_2d, uint32_t layers = 1, uint32_t base_mip_level = 0,
	                             uint32_t mip_levels = 1)
	    : m_extent{extent_2d.width, extent_2d.height, 1}, m_layers{layers}, m_base_mip_level{base_mip_level},
	      m_mip_levels{mip_levels} {}
	bool operator==(const SubImageSize &r) const {
		return std::tie(m_extent.width, m_extent.height, m_extent.depth, m_layers, m_base_mip_level, m_mip_levels) ==
		       std::tie(r.m_extent.width, r.m_extent.height, r.m_extent.depth, r.m_layers, r.m_base_mip_level,
		                r.m_mip_levels);
	}
	bool operator!=(const SubImageSize &r) const {
		return std::tie(m_extent.width, m_extent.height, m_extent.depth, m_layers, m_base_mip_level, m_mip_levels) !=
		       std::tie(r.m_extent.width, r.m_extent.height, r.m_extent.depth, r.m_layers, r.m_base_mip_level,
		                r.m_mip_levels);
	}

	inline const VkExtent3D &GetExtent() const { return m_extent; }
	inline uint32_t GetBaseMipLevel() const { return m_base_mip_level; }
	inline uint32_t GetMipLevels() const { return m_mip_levels; }
	inline uint32_t GetArrayLayers() const { return m_layers; }

	inline void Merge(const SubImageSize &r) {
		if (m_layers == 0)
			*this = r;
		else {
			assert(std::tie(m_extent.width, m_extent.height, m_extent.depth) ==
			       std::tie(r.m_extent.width, r.m_extent.height, r.m_extent.depth));

			if (m_layers == r.m_layers && m_base_mip_level + m_mip_levels == r.m_base_mip_level)
				m_mip_levels += r.m_mip_levels; // Merge MipMap
			else if (m_base_mip_level == r.m_base_mip_level && m_mip_levels == r.m_mip_levels)
				m_layers += r.m_layers;         // Merge Layer
			else
				assert(false);
		}
	}

	// TODO: Implement this
	// inline void Merge1D(const ImageSize &r) {}
	// inline void Merge2D(const ImageSize &r) {}
	// inline bool Merge3D(const ImageSize &r) {}
};

class ManagedImage final : public InternalImageBase,
                           public ImageAttachmentInfo<ManagedImage>,
                           public ManagedResourceInfo<ManagedImage, SubImageSize> {
private:
	VkImageViewType m_view_type{};
	VkFormat m_format{};

	friend class RenderGraphResolver;
	MYVK_RG_OBJECT_FRIENDS
	inline void Initialize(VkFormat format, VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D) {
		m_format = format;
		m_view_type = view_type;
		SetCanvasSize();
	}

public:
	inline constexpr ResourceState GetState() const { return ResourceState::kManaged; }
	inline constexpr ResourceClass GetClass() const { return ResourceClass::kManagedImage; }

	inline ManagedImage() : InternalImageBase(ResourceState::kManaged) {}
	inline ManagedImage(ManagedImage &&) noexcept = default;
	~ManagedImage() override = default;

	inline VkImageViewType GetViewType() const { return m_view_type; }
	inline VkFormat GetFormat() const { return m_format; }

	inline void SetSize2D(const VkExtent2D &extent_2d, uint32_t base_mip_level = 0, uint32_t mip_levels = 1) {
		SetSize(extent_2d, (uint32_t)1u, base_mip_level, mip_levels);
	}
	inline void SetSize2DArray(const VkExtent2D &extent_2d, uint32_t layer_count, uint32_t base_mip_level = 0,
	                           uint32_t mip_levels = 1) {
		SetSize(extent_2d, layer_count, base_mip_level, mip_levels);
	}
	inline void SetSize3D(const VkExtent3D &extent_3d, uint32_t base_mip_level = 0, uint32_t mip_levels = 1) {
		SetSize(extent_3d, (uint32_t)1u, base_mip_level, mip_levels);
	}
	inline void SetCanvasSize(uint32_t base_mip_level = 0, uint32_t mip_levels = 1) {
		SetSizeFunc([base_mip_level, mip_levels](const VkExtent2D &extent) {
			return SubImageSize{extent, (uint32_t)1u, base_mip_level, mip_levels};
		});
	}

	const myvk::Ptr<myvk::ImageView> &GetVkImageView() const;
};

class CombinedImage final : public InternalImageBase {
private:
	VkImageViewType m_view_type;
	std::vector<const ImageAlias *> m_images;

	friend class RenderGraphResolver;
	MYVK_RG_OBJECT_FRIENDS
	inline void Initialize(VkImageViewType view_type, std::vector<const ImageAlias *> &&images) {
		m_view_type = view_type;
		m_images = std::move(images);
	}
	template <typename Iterator>
	inline void Initialize(VkImageViewType view_type, Iterator images_begin, Iterator images_end) {
		m_view_type = view_type;
		m_images = {images_begin, images_end};
	}

public:
	inline constexpr ResourceState GetState() const { return ResourceState::kCombinedImage; }
	inline constexpr ResourceClass GetClass() const { return ResourceClass::kCombinedImage; }

	// For Each Direct Child ImageAlias
	template <typename Visitor> inline void ForEachImage(Visitor &&visitor) {
		for (auto *image : m_images)
			visitor(image);
	}
	template <typename Visitor> inline void ForEachImage(Visitor &&visitor) const {
		for (auto *image : m_images)
			visitor(image);
	}
	// For Each Direct Child ManagedImage, ImageAlias and CombinedImage, Expand ImageAlias
	template <typename Visitor> inline void ForEachExpandedImage(Visitor &&visitor) {
		for (auto *image : m_images)
			image->GetPointedResource()->Visit(visitor);
	}
	template <typename Visitor> inline void ForEachExpandedImage(Visitor &&visitor) const {
		for (auto *image : m_images)
			image->GetPointedResource()->Visit(visitor);
	}

	inline VkImageViewType GetViewType() const { return m_view_type; }
	inline VkFormat GetFormat() const { return m_images[0]->GetFormat(); }

	const myvk::Ptr<myvk::ImageView> &GetVkImageView() const;

	inline CombinedImage() : InternalImageBase(ResourceState::kCombinedImage) {}
	inline CombinedImage(CombinedImage &&) noexcept = default;
	inline const std::vector<const ImageAlias *> &GetImages() const { return m_images; }
	~CombinedImage() override = default;
};

// Last Frame Resources
template <typename Derived, typename VkType> class LastFrameResourceInfo {
public:
	using InitTransferFunc = std::function<void(const myvk::Ptr<myvk::CommandBuffer> &, const myvk::Ptr<VkType> &)>;

private:
	inline RenderGraphBase *get_render_graph_ptr() {
		static_assert(std::is_base_of_v<ObjectBase, Derived>);
		return static_cast<ObjectBase *>(static_cast<Derived *>(this))->GetRenderGraphPtr();
	}
	inline RenderGraphBase *get_render_graph_ptr() const {
		static_assert(std::is_base_of_v<ObjectBase, Derived>);
		return static_cast<const ObjectBase *>(static_cast<const Derived *>(this))->GetRenderGraphPtr();
	}

	InitTransferFunc m_init_transfer_func{};

public:
	inline void SetInitTransferFunc(const InitTransferFunc &func) {
		if ((m_init_transfer_func == nullptr) != (func == nullptr))
			get_render_graph_ptr()->SetCompilePhrases(CompilePhrase::kAllocate);
		else
			get_render_graph_ptr()->SetCompilePhrases(CompilePhrase::kInitLastFrameResource);

		m_init_transfer_func = func;
	}
	inline const InitTransferFunc &GetInitTransferFunc() const { return m_init_transfer_func; }
};

// Last Frame Resources
class LastFrameImage final : public ImageBase, public LastFrameResourceInfo<LastFrameImage, myvk::ImageBase> {
private:
	const InternalImageBase *m_pointed_image{};

	MYVK_RG_OBJECT_FRIENDS
	// Allow setting pointed_image later
	inline void Initialize() {}
	inline void Initialize(const InternalImageBase *image) { SetCurrentResource(image); }
	inline void Initialize(const ImageBase *image) { SetCurrentResource(image); }

public:
	inline constexpr ResourceState GetState() const { return ResourceState::kLastFrame; }
	inline constexpr ResourceClass GetClass() const { return ResourceClass::kLastFrameImage; }

	inline LastFrameImage() : ImageBase(ResourceState::kLastFrame) {}
	inline LastFrameImage(LastFrameImage &&) noexcept = default;
	inline ~LastFrameImage() final = default;

	inline void SetCurrentResource(const InternalImageBase *image) { m_pointed_image = image; }
	inline void SetCurrentResource(const ImageBase *image) {
		const auto get_internal_image = [](const auto *image) -> const InternalImageBase * {
			if constexpr (ResourceVisitorTrait<decltype(image)>::kIsInternal)
				return image;
			else {
				assert(false);
				return {};
			}
		};
		m_pointed_image = image->Visit([&get_internal_image](auto *image) -> const InternalImageBase * {
			if constexpr (ResourceVisitorTrait<decltype(image)>::kState == ResourceState::kAlias)
				return image->GetPointedResource()->Visit(get_internal_image);
			return image->Visit(get_internal_image);
		});
	}

	inline const InternalImageBase *GetCurrentResource() const { return m_pointed_image; }

	inline VkFormat GetFormat() const { return m_pointed_image->GetFormat(); }
	const myvk::Ptr<myvk::ImageView> &GetVkImageView() const;
};

class LastFrameBuffer final : public BufferBase, public LastFrameResourceInfo<LastFrameBuffer, myvk::BufferBase> {
private:
	const ManagedBuffer *m_pointed_buffer{};

	MYVK_RG_OBJECT_FRIENDS
	// Allow setting pointed_buffer later
	inline void Initialize() {}
	inline void Initialize(const ManagedBuffer *buffer) { SetCurrentResource(buffer); }
	inline void Initialize(const BufferBase *buffer) { SetCurrentResource(buffer); }

public:
	inline constexpr ResourceState GetState() const { return ResourceState::kLastFrame; }
	inline constexpr ResourceClass GetClass() const { return ResourceClass::kLastFrameBuffer; }

	inline LastFrameBuffer() : BufferBase(ResourceState::kLastFrame) {}
	inline LastFrameBuffer(LastFrameBuffer &&) = default;
	inline ~LastFrameBuffer() final = default;

	inline void SetCurrentResource(const ManagedBuffer *buffer) { m_pointed_buffer = buffer; }
	inline void SetCurrentResource(const BufferBase *buffer) {
		const auto get_managed_buffer = [](const auto *buffer) -> const ManagedBuffer * {
			if constexpr (ResourceVisitorTrait<decltype(buffer)>::kState == ResourceState::kManaged)
				return buffer;
			else {
				assert(false);
				return {};
			}
		};
		m_pointed_buffer = buffer->Visit([&get_managed_buffer](auto *buffer) -> const ManagedBuffer * {
			if constexpr (ResourceVisitorTrait<decltype(buffer)>::kState == ResourceState::kAlias)
				return buffer->GetPointedResource()->Visit(get_managed_buffer);
			else
				return buffer->Visit(get_managed_buffer);
		});
	}

	inline const ManagedBuffer *GetCurrentResource() const { return m_pointed_buffer; }

	const myvk::Ptr<myvk::BufferBase> &GetVkBuffer() const;
};

template <typename Visitor> std::invoke_result_t<Visitor, ManagedImage *> ResourceBase::Visit(Visitor &&visitor) {
	switch (m_class) {
	case ResourceClass::kManagedImage:
		return visitor(static_cast<ManagedImage *>(this));
	case ResourceClass::kExternalImageBase:
		return visitor(static_cast<ExternalImageBase *>(this));
	case ResourceClass::kCombinedImage:
		return visitor(static_cast<CombinedImage *>(this));
	case ResourceClass::kImageAlias:
		return visitor(static_cast<ImageAlias *>(this));
	case ResourceClass::kLastFrameImage:
		return visitor(static_cast<LastFrameImage *>(this));

	case ResourceClass::kManagedBuffer:
		return visitor(static_cast<ManagedBuffer *>(this));
	case ResourceClass::kExternalBufferBase:
		return visitor(static_cast<ExternalBufferBase *>(this));
	case ResourceClass::kBufferAlias:
		return visitor(static_cast<BufferAlias *>(this));
	case ResourceClass::kLastFrameBuffer:
		return visitor(static_cast<LastFrameBuffer *>(this));
	}
	assert(false);
	return visitor(static_cast<BufferAlias *>(nullptr));
}
template <typename Visitor> std::invoke_result_t<Visitor, ManagedImage *> ResourceBase::Visit(Visitor &&visitor) const {
	switch (m_class) {
	case ResourceClass::kManagedImage:
		return visitor(static_cast<const ManagedImage *>(this));
	case ResourceClass::kExternalImageBase:
		return visitor(static_cast<const ExternalImageBase *>(this));
	case ResourceClass::kCombinedImage:
		return visitor(static_cast<const CombinedImage *>(this));
	case ResourceClass::kImageAlias:
		return visitor(static_cast<const ImageAlias *>(this));
	case ResourceClass::kLastFrameImage:
		return visitor(static_cast<const LastFrameImage *>(this));

	case ResourceClass::kManagedBuffer:
		return visitor(static_cast<const ManagedBuffer *>(this));
	case ResourceClass::kExternalBufferBase:
		return visitor(static_cast<const ExternalBufferBase *>(this));
	case ResourceClass::kBufferAlias:
		return visitor(static_cast<const BufferAlias *>(this));
	case ResourceClass::kLastFrameBuffer:
		return visitor(static_cast<const LastFrameBuffer *>(this));
	}
	assert(false);
	return visitor(static_cast<const BufferAlias *>(nullptr));
}

template <typename Visitor> std::invoke_result_t<Visitor, ManagedImage *> ImageBase::Visit(Visitor &&visitor) {
	switch (GetState()) {
	case ResourceState::kManaged:
		return visitor(static_cast<ManagedImage *>(this));
	case ResourceState::kExternal:
		return visitor(static_cast<ExternalImageBase *>(this));
	case ResourceState::kCombinedImage:
		return visitor(static_cast<CombinedImage *>(this));
	case ResourceState::kAlias:
		return visitor(static_cast<ImageAlias *>(this));
	case ResourceState::kLastFrame:
		return visitor(static_cast<LastFrameImage *>(this));
	}
	assert(false);
	return visitor(static_cast<ImageAlias *>(nullptr));
}
template <typename Visitor> std::invoke_result_t<Visitor, ManagedImage *> ImageBase::Visit(Visitor &&visitor) const {
	switch (GetState()) {
	case ResourceState::kManaged:
		return visitor(static_cast<const ManagedImage *>(this));
	case ResourceState::kExternal:
		return visitor(static_cast<const ExternalImageBase *>(this));
	case ResourceState::kCombinedImage:
		return visitor(static_cast<const CombinedImage *>(this));
	case ResourceState::kAlias:
		return visitor(static_cast<const ImageAlias *>(this));
	case ResourceState::kLastFrame:
		return visitor(static_cast<const LastFrameImage *>(this));
	}
	assert(false);
	return visitor(static_cast<const ImageAlias *>(nullptr));
}

template <typename Visitor> std::invoke_result_t<Visitor, ManagedImage *> InternalImageBase::Visit(Visitor &&visitor) {
	switch (GetState()) {
	case ResourceState::kManaged:
		return visitor(static_cast<ManagedImage *>(this));
	case ResourceState::kCombinedImage:
		return visitor(static_cast<CombinedImage *>(this));
	default:
		assert(false);
		return visitor(static_cast<ManagedImage *>(nullptr));
	}
}
template <typename Visitor>
std::invoke_result_t<Visitor, ManagedImage *> InternalImageBase::Visit(Visitor &&visitor) const {
	switch (GetState()) {
	case ResourceState::kManaged:
		return visitor(static_cast<const ManagedImage *>(this));
	case ResourceState::kCombinedImage:
		return visitor(static_cast<const CombinedImage *>(this));
	default:
		assert(false);
		return visitor(static_cast<const ManagedImage *>(nullptr));
	}
}

template <typename Visitor> std::invoke_result_t<Visitor, ManagedBuffer *> BufferBase::Visit(Visitor &&visitor) {
	switch (GetState()) {
	case ResourceState::kManaged:
		return visitor(static_cast<ManagedBuffer *>(this));
	case ResourceState::kExternal:
		return visitor(static_cast<ExternalBufferBase *>(this));
	case ResourceState::kAlias:
		return visitor(static_cast<BufferAlias *>(this));
	case ResourceState::kLastFrame:
		return visitor(static_cast<LastFrameBuffer *>(this));
	default:
		assert(false);
	}
	return visitor(static_cast<BufferAlias *>(nullptr));
}
template <typename Visitor> std::invoke_result_t<Visitor, ManagedBuffer *> BufferBase::Visit(Visitor &&visitor) const {
	switch (GetState()) {
	case ResourceState::kManaged:
		return visitor(static_cast<const ManagedBuffer *>(this));
	case ResourceState::kExternal:
		return visitor(static_cast<const ExternalBufferBase *>(this));
	case ResourceState::kAlias:
		return visitor(static_cast<const BufferAlias *>(this));
	case ResourceState::kLastFrame:
		return visitor(static_cast<const LastFrameBuffer *>(this));
	default:
		assert(false);
	}
	return visitor(static_cast<const BufferAlias *>(nullptr));
}

template <typename Derived>
class ResourcePool : public Pool<Derived, PoolVariant<ManagedBuffer, ExternalBufferBase, LastFrameBuffer, ManagedImage,
                                                      CombinedImage, ExternalImageBase, LastFrameImage>> {
private:
	using _ResourcePool = Pool<Derived, PoolVariant<ManagedBuffer, ExternalBufferBase, LastFrameBuffer, ManagedImage,
	                                                CombinedImage, ExternalImageBase, LastFrameImage>>;

public:
	inline ResourcePool() = default;
	inline ResourcePool(ResourcePool &&) noexcept = default;
	inline ~ResourcePool() override = default;

protected:
	template <typename Type, typename... Args,
	          typename = std::enable_if_t<std::is_base_of_v<BufferBase, Type> || std::is_base_of_v<ImageBase, Type>>>
	inline Type *CreateResource(const PoolKey &resource_key, Args &&...args) {
		return _ResourcePool::template CreateAndInitialize<0, Type, Args...>(resource_key, std::forward<Args>(args)...);
	}
	template <typename Type, typename... Args,
	          typename = std::enable_if_t<std::is_base_of_v<BufferBase, Type> || std::is_base_of_v<ImageBase, Type>>>
	inline Type *CreateResourceForce(const PoolKey &resource_key, Args &&...args) {
		return _ResourcePool::template CreateAndInitializeForce<0, Type, Args...>(resource_key,
		                                                                          std::forward<Args>(args)...);
	}
	inline void DeleteResource(const PoolKey &resource_key) { return _ResourcePool::Delete(resource_key); }

	template <typename BufferType = BufferBase, typename = std::enable_if_t<std::is_base_of_v<BufferBase, BufferType> ||
	                                                                        std::is_same_v<BufferBase, BufferType>>>
	inline BufferType *GetBufferResource(const PoolKey &resource_buffer_key) const {
		return _ResourcePool::template Get<0, BufferType>(resource_buffer_key);
	}
	template <typename ImageType = ImageBase, typename = std::enable_if_t<std::is_base_of_v<ImageBase, ImageType> ||
	                                                                      std::is_same_v<ImageBase, ImageType>>>
	inline ImageType *GetImageResource(const PoolKey &resource_image_key) const {
		return _ResourcePool::template Get<0, ImageType>(resource_image_key);
	}
	template <typename ResourceType = ResourceBase,
	          typename = std::enable_if_t<std::is_base_of_v<ResourceBase, ResourceType> ||
	                                      std::is_same_v<ResourceBase, ResourceType>>>
	inline ResourceType *GetResource(const PoolKey &resource_image_key) const {
		return _ResourcePool::template Get<0, ResourceType>(resource_image_key);
	}
	inline void ClearResources() { _ResourcePool::Clear(); }
};

} // namespace myvk_rg::_details_

#endif

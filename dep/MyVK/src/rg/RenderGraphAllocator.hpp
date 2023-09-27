#ifndef MYVK_RG_RENDER_GRAPH_ALLOCATOR_HPP
#define MYVK_RG_RENDER_GRAPH_ALLOCATOR_HPP

#include <myvk_rg/_details_/Resource.hpp>

#include <myvk/BufferBase.hpp>
#include <myvk/ImageBase.hpp>
#include <myvk/ImageView.hpp>

#include "Bitset.hpp"
#include "RenderGraphResolver.hpp"

namespace myvk_rg::_details_ {

class RenderGraphAllocation;

class RenderGraphAllocator {
public:
	struct IntResourceAlloc {
		VkMemoryRequirements vk_memory_requirements{};
		VkDeviceSize memory_offset{}, db_memory_offset{};
		uint32_t allocation_id{};
		bool double_buffering{};

	protected:
		uint32_t internal_resource_id{};
		const RenderGraphResolver::IntResourceInfo *p_info{};
		friend class RenderGraphAllocator;
	};
	struct IntImageAlloc final : public IntResourceAlloc {
		myvk::Ptr<myvk::ImageBase> myvk_images[2]{};

		VkImageUsageFlags vk_image_usages{};
		VkImageType vk_image_type{VK_IMAGE_TYPE_2D};
		const SubImageSize *p_size{};

	protected:
		const RenderGraphResolver::IntImageInfo &GetImageInfo() const {
			return *static_cast<const RenderGraphResolver::IntImageInfo *>(p_info);
		}
		friend class RenderGraphAllocator;
		friend class RenderGraphLFInit;
	};
	struct IntBufferAlloc final : public IntResourceAlloc {
		myvk::Ptr<myvk::BufferBase> myvk_buffers[2]{};
		void *mapped_mem[2]{};

		VkBufferUsageFlags vk_buffer_usages{};

	protected:
		const RenderGraphResolver::IntBufferInfo &GetBufferInfo() const {
			return *static_cast<const RenderGraphResolver::IntBufferInfo *>(p_info);
		}
		friend class RenderGraphAllocator;
		friend class RenderGraphLFInit;
	};
	struct IntImageViewAlloc {
		const InternalImageBase *int_image;
		myvk::Ptr<myvk::ImageView> myvk_image_views[2]{};
		SubImageSize size{};
		uint32_t base_layer{};
	};

private:
	myvk::Ptr<myvk::Device> m_device_ptr;
	const RenderGraphResolver *m_p_resolved;

	std::vector<IntImageAlloc> m_allocated_images;
	std::vector<IntBufferAlloc> m_allocated_buffers;
	std::vector<IntImageViewAlloc> m_allocated_image_views;

	struct AllocationInfo {
		myvk::Ptr<RenderGraphAllocation> myvk_allocation{};
	};
	struct MemoryInfo {
		std::vector<IntResourceAlloc *> resources;
		VkDeviceSize alignment = 1;
		uint32_t memory_type_bits = -1;
		inline void push(IntResourceAlloc *resource) {
			resources.push_back(resource);
			alignment = std::max(alignment, resource->vk_memory_requirements.alignment);
			memory_type_bits &= resource->vk_memory_requirements.memoryTypeBits;
		}
		inline bool empty() const { return resources.empty(); }
	};
	std::vector<AllocationInfo> m_allocations;

	RelationMatrix m_allocated_resource_aliased_relation;

	void reset_resource_vectors();
	void _maintain_combined_image(const CombinedImage *image);
	void _accumulate_combined_image_base_layer(const CombinedImage *image);
	void update_resource_info();
	void create_vk_resources();
	void create_vk_image_views();
	void _make_naive_allocation(MemoryInfo &&memory_info, const VmaAllocationCreateInfo &allocation_create_info);
	void _make_optimal_allocation(MemoryInfo &&memory_info, const VmaAllocationCreateInfo &allocation_create_info);
	void create_and_bind_allocations();

public:
	void Allocate(const myvk::Ptr<myvk::Device> &device, const RenderGraphResolver &resolved);

	inline bool IsIntResourceAliased(uint32_t int_resource_id_0, uint32_t int_resource_id_1) const {
		return m_allocated_resource_aliased_relation.GetRelation(int_resource_id_0, int_resource_id_1);
	}
	template <typename Resource0, typename Resource1>
	inline bool IsIntResourceAliased(const Resource0 *resource_0, const Resource1 *resource_1) const {
		return m_allocated_resource_aliased_relation.GetRelation(m_p_resolved->GetIntResourceID(resource_0),
		                                                         m_p_resolved->GetIntResourceID(resource_1));
	}

	template <typename Resource> inline const IntResourceAlloc &GetIntResourceAlloc(const Resource *resource) const {
		if constexpr (ResourceTrait<Resource>::kIsImage)
			return GetIntImageAlloc(resource);
		else
			return GetIntBufferAlloc(resource);
	}
	template <typename Image> inline const IntImageAlloc &GetIntImageAlloc(const Image *image) const {
		return m_allocated_images[RenderGraphResolver::GetIntImageID(image)];
	}
	inline const IntImageAlloc &GetIntImageAlloc(uint32_t image_id) const { return m_allocated_images[image_id]; }
	inline const std::vector<IntImageAlloc> &GetIntImageAllocVector() const { return m_allocated_images; }

	template <typename Buffer> inline const IntBufferAlloc &GetIntBufferAlloc(const Buffer *buffer) const {
		return m_allocated_buffers[RenderGraphResolver::GetIntBufferID(buffer)];
	}
	inline const IntBufferAlloc &GetIntBufferAlloc(uint32_t buffer_id) const { return m_allocated_buffers[buffer_id]; }
	inline const std::vector<IntBufferAlloc> &GetIntBufferAllocVector() const { return m_allocated_buffers; }

	template <typename Image> inline const IntImageViewAlloc &GetIntImageViewAlloc(const Image *image) const {
		return m_allocated_image_views[RenderGraphResolver::GetIntImageViewID(image)];
	}
	inline const IntImageViewAlloc &GetIntImageViewAlloc(uint32_t image_view_id) const {
		return m_allocated_image_views[image_view_id];
	}
	inline const std::vector<IntImageViewAlloc> &GetIntImageViewAllocVector() const { return m_allocated_image_views; }

	inline const myvk::Ptr<myvk::ImageView> &GetVkImageView(const InternalImageBase *image, bool db) const {
		return m_allocated_image_views[RenderGraphResolver::GetIntImageViewID(image)].myvk_image_views[db];
	}
	inline const myvk::Ptr<myvk::ImageView> &GetVkImageView(const LastFrameImage *image, bool db) const {
		return m_allocated_image_views[RenderGraphResolver::GetIntImageViewID(image->GetCurrentResource())]
		    .myvk_image_views[!db];
	}
	inline static const myvk::Ptr<myvk::ImageView> &GetVkImageView(const ExternalImageBase *image, bool db) {
		return image->GetVkImageView();
	}
	inline const myvk::Ptr<myvk::ImageView> &GetVkImageView(const ImageAlias *image, bool db) const {
		return image->GetPointedResource()->Visit(
		    [this, db](const auto *image) -> const myvk::Ptr<myvk::ImageView> & { return GetVkImageView(image, db); });
	}
	inline const myvk::Ptr<myvk::ImageView> &GetVkImageView(const ImageBase *image, bool db) const {
		return image->Visit(
		    [this, db](const auto *image) -> const myvk::Ptr<myvk::ImageView> & { return GetVkImageView(image, db); });
	}

	inline const myvk::Ptr<myvk::ImageBase> &GetVkImage(const InternalImageBase *image, bool db) const {
		return m_allocated_images[RenderGraphResolver::GetIntImageID(image)].myvk_images[db];
	}
	inline const myvk::Ptr<myvk::ImageBase> &GetVkImage(const LastFrameImage *image, bool db) const {
		return m_allocated_images[RenderGraphResolver::GetIntImageID(image->GetCurrentResource())].myvk_images[!db];
	}
	inline static const myvk::Ptr<myvk::ImageBase> &GetVkImage(const ExternalImageBase *image, bool db) {
		return image->GetVkImageView()->GetImagePtr();
	}
	inline const myvk::Ptr<myvk::ImageBase> &GetVkImage(const ImageAlias *image, bool db) const {
		return image->GetPointedResource()->Visit(
		    [this, db](const auto *image) -> const myvk::Ptr<myvk::ImageBase> & { return GetVkImage(image, db); });
	}
	inline const myvk::Ptr<myvk::ImageBase> &GetVkImage(const ImageBase *image, bool db) const {
		return image->Visit(
		    [this, db](const auto *image) -> const myvk::Ptr<myvk::ImageBase> & { return GetVkImage(image, db); });
	}

	inline const myvk::Ptr<myvk::BufferBase> &GetVkBuffer(const ManagedBuffer *buffer, bool db) const {
		return m_allocated_buffers[RenderGraphResolver::GetIntBufferID(buffer)].myvk_buffers[db];
	}
	inline const myvk::Ptr<myvk::BufferBase> &GetVkBuffer(const LastFrameBuffer *buffer, bool db) const {
		return m_allocated_buffers[RenderGraphResolver::GetIntBufferID(buffer->GetCurrentResource())].myvk_buffers[!db];
	}
	inline static const myvk::Ptr<myvk::BufferBase> &GetVkBuffer(const ExternalBufferBase *buffer, bool db) {
		return buffer->GetVkBuffer();
	}
	inline const myvk::Ptr<myvk::BufferBase> &GetVkBuffer(const BufferAlias *buffer, bool db) const {
		return buffer->GetPointedResource()->Visit(
		    [this, db](const auto *buffer) -> const myvk::Ptr<myvk::BufferBase> & { return GetVkBuffer(buffer, db); });
	}
	inline const myvk::Ptr<myvk::BufferBase> &GetVkBuffer(const BufferBase *buffer, bool db) const {
		return buffer->Visit(
		    [this, db](const auto *buffer) -> const myvk::Ptr<myvk::BufferBase> & { return GetVkBuffer(buffer, db); });
	}

	inline void *GetMappedData(const ManagedBuffer *buffer, bool db) const {
		return m_allocated_buffers[RenderGraphResolver::GetIntBufferID(buffer)].mapped_mem[db];
	}
};

} // namespace myvk_rg::_details_

#endif

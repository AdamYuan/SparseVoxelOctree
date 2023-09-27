#include "RenderGraphAllocator.hpp"

#include "VkHelper.hpp"

#include <algorithm>

#ifdef MYVK_RG_DEBUG
#include <iostream>
#endif

namespace myvk_rg::_details_ {

// TODO: Alloc Double-Buffering Images
class RenderGraphImage final : public myvk::ImageBase {
private:
	myvk::Ptr<myvk::Device> m_device_ptr;

public:
	inline RenderGraphImage(const myvk::Ptr<myvk::Device> &device, const VkImageCreateInfo &create_info)
	    : m_device_ptr{device} {
		vkCreateImage(GetDevicePtr()->GetHandle(), &create_info, nullptr, &m_image);
		m_extent = create_info.extent;
		m_mip_levels = create_info.mipLevels;
		m_array_layers = create_info.arrayLayers;
		m_format = create_info.format;
		m_type = create_info.imageType;
		m_usage = create_info.usage;
	}
	inline ~RenderGraphImage() final {
		if (m_image != VK_NULL_HANDLE)
			vkDestroyImage(GetDevicePtr()->GetHandle(), m_image, nullptr);
	};
	const myvk::Ptr<myvk::Device> &GetDevicePtr() const final { return m_device_ptr; }
};
class RenderGraphBuffer final : public myvk::BufferBase {
private:
	myvk::Ptr<myvk::Device> m_device_ptr;

public:
	inline RenderGraphBuffer(const myvk::Ptr<myvk::Device> &device, const VkBufferCreateInfo &create_info)
	    : m_device_ptr{device} {
		vkCreateBuffer(GetDevicePtr()->GetHandle(), &create_info, nullptr, &m_buffer);
		m_size = create_info.size;
	}
	inline ~RenderGraphBuffer() final {
		if (m_buffer != VK_NULL_HANDLE)
			vkDestroyBuffer(GetDevicePtr()->GetHandle(), m_buffer, nullptr);
	}
	const myvk::Ptr<myvk::Device> &GetDevicePtr() const final { return m_device_ptr; }
};

class RenderGraphAllocation final : public myvk::DeviceObjectBase {
private:
	myvk::Ptr<myvk::Device> m_device_ptr;
	VmaAllocation m_allocation{VK_NULL_HANDLE};
	VmaAllocationInfo m_info{};

public:
	inline RenderGraphAllocation(const myvk::Ptr<myvk::Device> &device, const VkMemoryRequirements &memory_requirements,
	                             const VmaAllocationCreateInfo &create_info)
	    : m_device_ptr{device} {
		vmaAllocateMemory(device->GetAllocatorHandle(), &memory_requirements, &create_info, &m_allocation, &m_info);
		if (create_info.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) {
			assert(m_info.pMappedData);
		}
	}
	inline ~RenderGraphAllocation() final {
		if (m_allocation != VK_NULL_HANDLE)
			vmaFreeMemory(GetDevicePtr()->GetAllocatorHandle(), m_allocation);
	}
	inline const VmaAllocationInfo &GetInfo() const { return m_info; }
	inline VmaAllocation GetHandle() const { return m_allocation; }
	const myvk::Ptr<myvk::Device> &GetDevicePtr() const final { return m_device_ptr; }
};

void RenderGraphAllocator::_maintain_combined_image(const CombinedImage *image) {
	// Visit Each Child Image, Update Size and Base Layer (Relative, need to be accumulated after)
	SubImageSize &cur_size = m_allocated_image_views[RenderGraphResolver::GetIntImageViewID(image)].size;
	image->ForEachExpandedImage([this, &cur_size](auto *sub_image) -> void {
		if constexpr (ResourceVisitorTrait<decltype(sub_image)>::kIsCombinedOrManagedImage) {
			// Merge the Size of the Current Child Image
			auto &sub_image_alloc = m_allocated_image_views[RenderGraphResolver::GetIntImageViewID(sub_image)];
			if constexpr (ResourceVisitorTrait<decltype(sub_image)>::kClass == ResourceClass::kManagedImage)
				sub_image_alloc.size = sub_image->GetSize();
			else if constexpr (ResourceVisitorTrait<decltype(sub_image)>::kClass == ResourceClass::kCombinedImage)
				_maintain_combined_image(sub_image); // Further Query SubImage Size

			cur_size.Merge(sub_image_alloc.size);
			sub_image_alloc.base_layer = cur_size.GetArrayLayers() - sub_image_alloc.size.GetArrayLayers();
		}
	});
}

void RenderGraphAllocator::_accumulate_combined_image_base_layer(const CombinedImage *image) {
	uint32_t cur_base_layer = m_allocated_image_views[RenderGraphResolver::GetIntImageViewID(image)].base_layer;
	image->ForEachExpandedImage([this, cur_base_layer](auto *sub_image) -> void {
		if constexpr (ResourceVisitorTrait<decltype(sub_image)>::kIsCombinedOrManagedImage) {
			auto &sub_image_alloc = m_allocated_image_views[RenderGraphResolver::GetIntImageViewID(sub_image)];
			sub_image_alloc.base_layer += cur_base_layer;
			if constexpr (ResourceVisitorTrait<decltype(sub_image)>::kClass == ResourceClass::kCombinedImage)
				_accumulate_combined_image_base_layer(sub_image);
		}
	});
}

void RenderGraphAllocator::update_resource_info() {
	// Update VkBufferUsage
	for (auto &buffer_alloc : m_allocated_buffers) {
		const auto &buffer_info = buffer_alloc.GetBufferInfo();
		// buffer_alloc.vk_buffer_usages = buffer_info.buffer->GetExtraUsages();
		buffer_alloc.vk_buffer_usages = buffer_info.p_last_frame_info && static_cast<const LastFrameBuffer *>(
		                                                                     buffer_info.p_last_frame_info->lf_resource)
		                                                                     ->GetInitTransferFunc()
		                                    ? VK_BUFFER_USAGE_TRANSFER_DST_BIT
		                                    : 0;
		for (const auto &ref : buffer_info.references)
			buffer_alloc.vk_buffer_usages |= UsageGetCreationUsages(ref.p_input->GetUsage());
	}
	// Update Image Sizes, Base Layers and VkImageUsage
	for (auto &image_alloc : m_allocated_images) {
		const auto &image_info = image_alloc.GetImageInfo();
		// image_alloc.persistence = false;

		auto &image_view_alloc = m_allocated_image_views[RenderGraphResolver::GetIntImageViewID(image_info.image)];
		image_view_alloc.base_layer = 0;

		image_info.image->Visit([this, &image_alloc, &image_view_alloc](auto *image) -> void {
			if constexpr (ResourceVisitorTrait<decltype(image)>::kIsInternal) {
				if constexpr (ResourceVisitorTrait<decltype(image)>::kClass == ResourceClass::kCombinedImage) {
					_maintain_combined_image(image);
					_accumulate_combined_image_base_layer(image);
				} else
					image_view_alloc.size = image->GetSize();
				image_alloc.p_size = &image_view_alloc.size;
			} else
				assert(false);
		});

		image_alloc.vk_image_usages = image_info.p_last_frame_info && static_cast<const LastFrameImage *>(
		                                                                  image_info.p_last_frame_info->lf_resource)
		                                                                  ->GetInitTransferFunc()
		                                  ? VK_IMAGE_USAGE_TRANSFER_DST_BIT
		                                  : 0;
		image_alloc.vk_image_type = VK_IMAGE_TYPE_2D;
		for (const auto &ref : image_info.references)
			image_alloc.vk_image_usages |= UsageGetCreationUsages(ref.p_input->GetUsage());
		if (image_info.p_last_frame_info)
			for (const auto &ref : image_info.p_last_frame_info->references)
				image_alloc.vk_image_usages |= UsageGetCreationUsages(ref.p_input->GetUsage());
	}

	// Update VkImageType and VkImageUsages
	for (uint32_t image_view_id = 0; image_view_id < m_p_resolved->GetIntImageViewCount(); ++image_view_id) {
		const auto &image_view_info = m_p_resolved->GetIntImageViewInfo(image_view_id);
		image_view_info.image->Visit([this](const auto *image) {
			using Trait = ResourceVisitorTrait<decltype(image)>;
			if constexpr (Trait::kIsInternal) {
				auto &image_alloc = m_allocated_images[m_p_resolved->GetIntImageID(image)];
				UpdateVkImageTypeFromVkImageViewType(&image_alloc.vk_image_type, image->GetViewType());
				// if constexpr (Trait::kState == ResourceState::kManaged)
				// 	image_alloc.vk_image_usages |= image->GetExtraUsages();
			} else
				assert(false);
		});
	}

	// Check double_buffering
	const auto check_double_buffering = [this](const RenderGraphResolver::IntResourceInfo &int_res_info) -> bool {
		if (int_res_info.p_last_frame_info == nullptr || int_res_info.validation_references.empty())
			return false;

		const auto last_frame_references = int_res_info.p_last_frame_info->last_references;
		for (uint32_t i = last_frame_references.size() - 1; ~i; --i) {
			for (const auto &validation_ref : int_res_info.validation_references)
				if (!m_p_resolved->IsPassPrior(last_frame_references[i].pass, validation_ref.pass))
					return true;
		}
		return false;
	};

	for (auto &buffer_alloc : m_allocated_buffers)
		buffer_alloc.double_buffering = check_double_buffering(buffer_alloc.GetBufferInfo());

	for (auto &image_alloc : m_allocated_images)
		image_alloc.double_buffering = check_double_buffering(image_alloc.GetImageInfo());
}

void RenderGraphAllocator::create_vk_resources() {
	// Create Buffers
	for (auto &buffer_alloc : m_allocated_buffers) {
		const auto &buffer_info = buffer_alloc.GetBufferInfo();
		VkBufferCreateInfo create_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		create_info.usage = buffer_alloc.vk_buffer_usages;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.size = buffer_info.buffer->GetSize();

		buffer_alloc.myvk_buffers[0] = std::make_shared<RenderGraphBuffer>(m_device_ptr, create_info);
		vkGetBufferMemoryRequirements(m_device_ptr->GetHandle(), buffer_alloc.myvk_buffers[0]->GetHandle(),
		                              &buffer_alloc.vk_memory_requirements);

		if (buffer_alloc.double_buffering)
			buffer_alloc.myvk_buffers[1] = std::make_shared<RenderGraphBuffer>(m_device_ptr, create_info);
		else
			buffer_alloc.myvk_buffers[1] = buffer_alloc.myvk_buffers[0];
	}

	// Create Images
	for (auto &image_alloc : m_allocated_images) {
		const auto &image_info = image_alloc.GetImageInfo();
		assert(image_alloc.p_size && image_alloc.p_size->GetBaseMipLevel() == 0);

		VkImageCreateInfo create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
		create_info.usage = image_alloc.vk_image_usages;
		if (image_info.is_transient)
			create_info.usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		create_info.format = image_info.image->GetFormat();
		create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		create_info.imageType = image_alloc.vk_image_type;
		{ // Set Size Info
			VkExtent3D &extent = create_info.extent;
			extent = {1, 1, 1};
			switch (create_info.imageType) {
			case VK_IMAGE_TYPE_1D: {
				extent.width = image_alloc.p_size->GetExtent().width;
				create_info.mipLevels = image_alloc.p_size->GetMipLevels();
				create_info.arrayLayers = image_alloc.p_size->GetArrayLayers();
			} break;
			case VK_IMAGE_TYPE_2D: {
				extent.width = image_alloc.p_size->GetExtent().width;
				extent.height = image_alloc.p_size->GetExtent().height;
				create_info.mipLevels = image_alloc.p_size->GetMipLevels();
				create_info.arrayLayers = image_alloc.p_size->GetArrayLayers();
			} break;
			case VK_IMAGE_TYPE_3D: {
				assert(image_alloc.p_size->GetExtent().depth == 1 || image_alloc.p_size->GetArrayLayers() == 1);
				extent.width = image_alloc.p_size->GetExtent().width;
				extent.height = image_alloc.p_size->GetExtent().height;
				extent.depth = std::max(image_alloc.p_size->GetExtent().depth, image_alloc.p_size->GetArrayLayers());
				create_info.mipLevels = image_alloc.p_size->GetMipLevels();
				create_info.arrayLayers = 1;
			} break;
			default:
				assert(false);
			}
		}

		image_alloc.myvk_images[0] = std::make_shared<RenderGraphImage>(m_device_ptr, create_info);
		vkGetImageMemoryRequirements(m_device_ptr->GetHandle(), image_alloc.myvk_images[0]->GetHandle(),
		                             &image_alloc.vk_memory_requirements);

		if (image_alloc.double_buffering)
			image_alloc.myvk_images[1] = std::make_shared<RenderGraphImage>(m_device_ptr, create_info);
		else
			image_alloc.myvk_images[1] = image_alloc.myvk_images[0];
	}
}

void RenderGraphAllocator::create_vk_image_views() {
#ifdef MYVK_RG_DEBUG
	printf("\nImage Views: \n");
#endif
	for (uint32_t image_view_id = 0; image_view_id < m_p_resolved->GetIntImageViewCount(); ++image_view_id) {
		const auto *image = m_p_resolved->GetIntImageViewInfo(image_view_id).image;
		auto &image_view_alloc = m_allocated_image_views[image_view_id];
		image_view_alloc.int_image = image;
		image->Visit([this, &image_view_alloc](const auto *image) {
			if constexpr (ResourceVisitorTrait<decltype(image)>::kIsInternal) {
				VkImageViewCreateInfo create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
				create_info.format = image->GetFormat();
				create_info.viewType = image->GetViewType();
				create_info.subresourceRange.baseArrayLayer = image_view_alloc.base_layer;
				create_info.subresourceRange.layerCount = image_view_alloc.size.GetArrayLayers();
				create_info.subresourceRange.baseMipLevel = image_view_alloc.size.GetBaseMipLevel();
				create_info.subresourceRange.levelCount = image_view_alloc.size.GetMipLevels();
				create_info.subresourceRange.aspectMask = VkImageAspectFlagsFromVkFormat(image->GetFormat());

				uint32_t image_id = m_p_resolved->GetIntImageID(image);

				image_view_alloc.myvk_image_views[0] =
				    myvk::ImageView::Create(m_allocated_images[image_id].myvk_images[0], create_info);

				if (m_allocated_images[image_id].double_buffering)
					image_view_alloc.myvk_image_views[1] =
					    myvk::ImageView::Create(m_allocated_images[image_id].myvk_images[1], create_info);
				else
					image_view_alloc.myvk_image_views[1] = image_view_alloc.myvk_image_views[0];

#ifdef MYVK_RG_DEBUG
				std::cout << image->GetKey().GetName() << ":" << image->GetKey().GetID();
				printf(" {baseArrayLayer, layerCount, baseMipLevel, levelCount} = {%u, %u, %u, %u}\n",
				       create_info.subresourceRange.baseArrayLayer, create_info.subresourceRange.layerCount,
				       create_info.subresourceRange.baseMipLevel, create_info.subresourceRange.levelCount);
#endif
			} else
				assert(false);
		});
	}
#ifdef MYVK_RG_DEBUG
	printf("\n");
#endif
}

inline static constexpr VkDeviceSize DivRoundUp(VkDeviceSize l, VkDeviceSize r) { return (l / r) + (l % r ? 1 : 0); }

void RenderGraphAllocator::_make_naive_allocation(MemoryInfo &&memory_info,
                                                  const VmaAllocationCreateInfo &allocation_create_info) {
	if (memory_info.empty())
		return;

	uint32_t allocation_id = m_allocations.size();
	m_allocations.emplace_back();
	auto &allocation_info = m_allocations.back();

	VkMemoryRequirements memory_requirements = {};
	memory_requirements.alignment = memory_info.alignment;
	memory_requirements.memoryTypeBits = memory_info.memory_type_bits;

	VkDeviceSize allocation_blocks = 0;
	for (auto *p_resource_info : memory_info.resources) {
		p_resource_info->allocation_id = allocation_id;
		p_resource_info->memory_offset = allocation_blocks * memory_info.alignment;
		allocation_blocks += DivRoundUp(p_resource_info->vk_memory_requirements.size, memory_info.alignment);

		if (p_resource_info->double_buffering) {
			p_resource_info->db_memory_offset = allocation_blocks * memory_info.alignment;
			allocation_blocks += DivRoundUp(p_resource_info->vk_memory_requirements.size, memory_info.alignment);
		}
	}
	memory_requirements.size = allocation_blocks * memory_info.alignment;

	allocation_info.myvk_allocation =
	    std::make_shared<RenderGraphAllocation>(m_device_ptr, memory_requirements, allocation_create_info);
}

// An AABB indicates a placed resource
struct MemBlock {
	VkDeviceSize mem_begin, mem_end;
	uint32_t internal_resource_id;
};
struct MemEvent {
	VkDeviceSize mem;
	uint32_t cnt;
	inline bool operator<(const MemEvent &r) const { return mem < r.mem; }
};
void RenderGraphAllocator::_make_optimal_allocation(MemoryInfo &&memory_info,
                                                    const VmaAllocationCreateInfo &allocation_create_info) {
	if (memory_info.empty())
		return;

	uint32_t allocation_id = m_allocations.size();
	m_allocations.emplace_back();
	auto &allocation_info = m_allocations.back();

	// Sort Resources by required sizes, place large resources first
	std::sort(memory_info.resources.begin(), memory_info.resources.end(),
	          [](const IntResourceAlloc *l, const IntResourceAlloc *r) -> bool {
		          return l->vk_memory_requirements.size > r->vk_memory_requirements.size ||
		                 (l->vk_memory_requirements.size == r->vk_memory_requirements.size &&
		                  RenderGraphResolver::GetPassOrder(l->p_info->references[0].pass) <
		                      RenderGraphResolver::GetPassOrder(r->p_info->references[0].pass));
	          });

	VkDeviceSize allocation_blocks = 0;
	{
		std::vector<MemBlock> blocks;
		std::vector<MemEvent> events;
		blocks.reserve(memory_info.resources.size());
		events.reserve(memory_info.resources.size() << 1u);

		for (auto *p_resource_info : memory_info.resources) {
			// Find an empty position to place
			events.clear();
			for (const auto &block : blocks)
				if (m_p_resolved->IsIntResourceConflicted(p_resource_info->internal_resource_id,
				                                          block.internal_resource_id)) {
					events.push_back({block.mem_begin, 1});
					events.push_back({block.mem_end, (uint32_t)-1});
				}
			std::sort(events.begin(), events.end());

			VkDeviceSize required_mem_size =
			    DivRoundUp(p_resource_info->vk_memory_requirements.size, memory_info.alignment);

			VkDeviceSize optimal_mem_pos = 0, optimal_mem_size = std::numeric_limits<VkDeviceSize>::max();
			if (!events.empty()) {
				assert(events.front().cnt == 1 && events.back().cnt == -1);
				if (events.front().mem >= required_mem_size)
					optimal_mem_size = events.front().mem;
				else
					optimal_mem_pos = events.back().mem;

				for (uint32_t i = 1; i < events.size(); ++i) {
					events[i].cnt += events[i - 1].cnt;
					if (events[i - 1].cnt == 0 && events[i].cnt == 1) {
						VkDeviceSize cur_mem_pos = events[i - 1].mem, cur_mem_size = events[i].mem - events[i - 1].mem;
						if (required_mem_size <= cur_mem_size && cur_mem_size < optimal_mem_size) {
							optimal_mem_size = cur_mem_size;
							optimal_mem_pos = cur_mem_pos;
						}
					}
				}
			}

			p_resource_info->allocation_id = allocation_id;
			p_resource_info->memory_offset = optimal_mem_pos * memory_info.alignment;
			allocation_blocks = std::max(allocation_blocks, optimal_mem_pos + required_mem_size);

			blocks.push_back(
			    {optimal_mem_pos, optimal_mem_pos + required_mem_size, p_resource_info->internal_resource_id});
		}
	}

	// Extract resource conflicts
	for (uint32_t i = 0; i < memory_info.resources.size(); ++i) {
		const auto &resource_info_i = memory_info.resources[i];
		for (uint32_t j = 0; j < i; ++j) {
			const auto &resource_info_j = memory_info.resources[j];
			if (resource_info_i->memory_offset + resource_info_i->vk_memory_requirements.size >
			        resource_info_j->memory_offset &&
			    resource_info_j->memory_offset + resource_info_j->vk_memory_requirements.size >
			        resource_info_i->memory_offset) {
				m_allocated_resource_aliased_relation.SetRelation(resource_info_i->internal_resource_id,
				                                                  resource_info_j->internal_resource_id);
				m_allocated_resource_aliased_relation.SetRelation(resource_info_j->internal_resource_id,
				                                                  resource_info_i->internal_resource_id);
			}
		}
	}

	VkMemoryRequirements memory_requirements = {};
	memory_requirements.alignment = memory_info.alignment;
	memory_requirements.memoryTypeBits = memory_info.memory_type_bits;
	memory_requirements.size = allocation_blocks * memory_info.alignment;

	allocation_info.myvk_allocation =
	    std::make_shared<RenderGraphAllocation>(m_device_ptr, memory_requirements, allocation_create_info);
}

void RenderGraphAllocator::create_and_bind_allocations() {
	m_allocations.clear();
	m_allocated_resource_aliased_relation.Reset(m_p_resolved->GetIntResourceCount(),
	                                            m_p_resolved->GetIntResourceCount());

	{
		// Create Allocations
		MemoryInfo device_memory{}, persistent_device_memory{}, lazy_memory{}, //
		    rnd_mapped_memory{}, seq_mapped_memory{};

		device_memory.resources.reserve(m_p_resolved->GetIntResourceCount());
		uint32_t buffer_image_granularity =
		    m_device_ptr->GetPhysicalDevicePtr()->GetProperties().vk10.limits.bufferImageGranularity;
		device_memory.alignment = persistent_device_memory.alignment = buffer_image_granularity;

		for (auto &image_alloc : m_allocated_images) {
			const auto &image_info = image_alloc.GetImageInfo();
			if (image_info.is_transient)
				lazy_memory.push(&image_alloc); // If the image is Transient and LAZY_ALLOCATION is supported
			else if (image_info.p_last_frame_info)
				persistent_device_memory.push(&image_alloc);
			else
				device_memory.push(&image_alloc);
		}
		for (auto &buffer_alloc : m_allocated_buffers) {
			const auto &buffer_info = buffer_alloc.GetBufferInfo();
			auto map_type = buffer_info.buffer->GetMapType();
			if (map_type != BufferMapType::kNone) {
				if (map_type == BufferMapType::kRandom)
					rnd_mapped_memory.push(&buffer_alloc);
				else if (map_type == BufferMapType::kSeqWrite)
					seq_mapped_memory.push(&buffer_alloc);
			} else if (buffer_info.p_last_frame_info)
				persistent_device_memory.push(&buffer_alloc);
			else
				device_memory.push(&buffer_alloc);
		}
		{
			VmaAllocationCreateInfo create_info = {};
			create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT;
			create_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			_make_optimal_allocation(std::move(device_memory), create_info);
		}
		{
			VmaAllocationCreateInfo create_info = {};
			create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			create_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			_make_naive_allocation(std::move(persistent_device_memory), create_info);
		}
		{
			VmaAllocationCreateInfo create_info = {};
			create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			create_info.usage = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
			_make_naive_allocation(std::move(lazy_memory), create_info);
		}
		{
			VmaAllocationCreateInfo create_info = {};
			create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT |
			                    VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			create_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			_make_naive_allocation(std::move(rnd_mapped_memory), create_info);
		}
		{
			VmaAllocationCreateInfo create_info = {};
			create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT |
			                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			create_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			_make_naive_allocation(std::move(seq_mapped_memory), create_info);
		}
	}
	// Bind Memory
	for (auto &image_alloc : m_allocated_images) {
		const auto &allocation_ptr = m_allocations[image_alloc.allocation_id].myvk_allocation;
		vmaBindImageMemory2(m_device_ptr->GetAllocatorHandle(), allocation_ptr->GetHandle(), image_alloc.memory_offset,
		                    image_alloc.myvk_images[0]->GetHandle(), nullptr);
		if (image_alloc.double_buffering)
			vmaBindImageMemory2(m_device_ptr->GetAllocatorHandle(), allocation_ptr->GetHandle(),
			                    image_alloc.db_memory_offset, image_alloc.myvk_images[1]->GetHandle(), nullptr);
	}
	for (auto &buffer_alloc : m_allocated_buffers) {
		const auto &allocation_ptr = m_allocations[buffer_alloc.allocation_id].myvk_allocation;
		vmaBindBufferMemory2(m_device_ptr->GetAllocatorHandle(), allocation_ptr->GetHandle(),
		                     buffer_alloc.memory_offset, buffer_alloc.myvk_buffers[0]->GetHandle(), nullptr);
		buffer_alloc.mapped_mem[0] = (uint8_t *)allocation_ptr->GetInfo().pMappedData + buffer_alloc.memory_offset;
		if (buffer_alloc.double_buffering) {
			vmaBindBufferMemory2(m_device_ptr->GetAllocatorHandle(), allocation_ptr->GetHandle(),
			                     buffer_alloc.db_memory_offset, buffer_alloc.myvk_buffers[1]->GetHandle(), nullptr);
			buffer_alloc.mapped_mem[1] =
			    (uint8_t *)allocation_ptr->GetInfo().pMappedData + buffer_alloc.db_memory_offset;
		} else
			buffer_alloc.mapped_mem[1] = buffer_alloc.mapped_mem[0];
	}

#ifdef MYVK_RG_DEBUG
	printf("Aliased:\n");
	for (uint32_t i = 0; i < m_p_resolved->GetIntResourceCount(); ++i) {
		for (uint32_t j = 0; j < m_p_resolved->GetIntResourceCount(); ++j)
			printf("%d ", m_allocated_resource_aliased_relation.GetRelation(i, j));
		printf("\n");
	}

	printf("\nAllocations: \n");
	for (const auto &allocation_info : m_allocations) {
		VmaAllocationInfo info;
		vmaGetAllocationInfo(m_device_ptr->GetAllocatorHandle(), allocation_info.myvk_allocation->GetHandle(), &info);
		VkMemoryPropertyFlags flags;
		vmaGetAllocationMemoryProperties(m_device_ptr->GetAllocatorHandle(),
		                                 allocation_info.myvk_allocation->GetHandle(), &flags);
		printf("allocation: size = %lu MB, memory_type = %u\n", info.size >> 20u, flags);
	}
	printf("\n");
#endif
}

void RenderGraphAllocator::reset_resource_vectors() {
	m_allocated_images.clear();
	m_allocated_image_views.clear();
	m_allocated_buffers.clear();

	m_allocated_images.resize(m_p_resolved->GetIntImageCount());
	m_allocated_image_views.resize(m_p_resolved->GetIntImageViewCount());
	m_allocated_buffers.resize(m_p_resolved->GetIntBufferCount());

	for (uint32_t buffer_id = 0; buffer_id < m_p_resolved->GetIntBufferCount(); ++buffer_id) {
		const auto &buffer_info = m_p_resolved->GetIntBufferInfo(buffer_id);
		auto &buffer_alloc = m_allocated_buffers[buffer_id];
		buffer_alloc.internal_resource_id = m_p_resolved->GetIntResourceID(buffer_info.buffer);
		buffer_alloc.p_info = &buffer_info;
	}

	// Create Images
	for (uint32_t image_id = 0; image_id < m_p_resolved->GetIntImageCount(); ++image_id) {
		const auto &image_info = m_p_resolved->GetIntImageInfo(image_id);
		auto &image_alloc = m_allocated_images[image_id];
		image_alloc.internal_resource_id = RenderGraphResolver::GetIntResourceID(image_info.image);
		image_alloc.p_info = &image_info;
	}
}

void RenderGraphAllocator::Allocate(const myvk::Ptr<myvk::Device> &device, const RenderGraphResolver &resolved) {
	m_device_ptr = device;
	m_p_resolved = &resolved;

	reset_resource_vectors();
	update_resource_info();
	create_vk_resources();
	create_and_bind_allocations();
	create_vk_image_views();

#ifdef MYVK_RG_DEBUG
	printf("\nImages: \n");
	for (uint32_t i = 0; i < m_p_resolved->GetIntImageCount(); ++i) {
		const auto &image_info = m_p_resolved->GetIntImageInfo(i);
		const auto &image_alloc = m_allocated_images[i];
		std::cout << image_info.image->GetKey().GetName() << ":" << image_info.image->GetKey().GetID()
		          << " mip_levels = " << image_alloc.p_size->GetMipLevels()
		          << " usage = " << image_alloc.vk_image_usages << " {size, alignment, flag} = {"
		          << image_alloc.vk_memory_requirements.size << ", " << image_alloc.vk_memory_requirements.alignment
		          << ", " << image_alloc.vk_memory_requirements.memoryTypeBits << "}"
		          << " transient = " << image_info.is_transient << " offset = " << image_alloc.memory_offset
		          << " double_buffering = " << image_alloc.double_buffering << std::endl;
	}
	printf("\n");

	printf("\nBuffers: \n");
	for (uint32_t i = 0; i < m_p_resolved->GetIntBufferCount(); ++i) {
		const auto &buffer_info = m_p_resolved->GetIntBufferInfo(i);
		const auto &buffer_alloc = m_allocated_buffers[i];
		std::cout << buffer_info.buffer->GetKey().GetName() << ":" << buffer_info.buffer->GetKey().GetID()
		          << " {size, alignment, flag} = {" << buffer_alloc.vk_memory_requirements.size << ", "
		          << buffer_alloc.vk_memory_requirements.alignment << ", "
		          << buffer_alloc.vk_memory_requirements.memoryTypeBits << "}"
		          << " offset = " << buffer_alloc.memory_offset
		          << " double_buffering = " << buffer_alloc.double_buffering << std::endl;
	}
	printf("\n");
#endif
}

} // namespace myvk_rg::_details_

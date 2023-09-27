#ifndef MYVK_RG_RESOURCE_BASE_HPP
#define MYVK_RG_RESOURCE_BASE_HPP

#include "ObjectBase.hpp"
#include <cinttypes>
#include <type_traits>

namespace myvk_rg::_details_ {

// Resource Base and Types
enum class ResourceType : uint8_t { kImage, kBuffer };
enum class ResourceState : uint8_t { kManaged, kCombinedImage, kExternal, kAlias, kLastFrame };
#define MAKE_RESOURCE_CLASS_VAL(Type, State) uint8_t(static_cast<uint8_t>(State) << 1u | static_cast<uint8_t>(Type))
enum class ResourceClass : uint8_t {
	kManagedImage = MAKE_RESOURCE_CLASS_VAL(ResourceType::kImage, ResourceState::kManaged),
	kExternalImageBase = MAKE_RESOURCE_CLASS_VAL(ResourceType::kImage, ResourceState::kExternal),
	kCombinedImage = MAKE_RESOURCE_CLASS_VAL(ResourceType::kImage, ResourceState::kCombinedImage),
	kImageAlias = MAKE_RESOURCE_CLASS_VAL(ResourceType::kImage, ResourceState::kAlias),
	kLastFrameImage = MAKE_RESOURCE_CLASS_VAL(ResourceType::kImage, ResourceState::kLastFrame),

	kManagedBuffer = MAKE_RESOURCE_CLASS_VAL(ResourceType::kBuffer, ResourceState::kManaged),
	kExternalBufferBase = MAKE_RESOURCE_CLASS_VAL(ResourceType::kBuffer, ResourceState::kExternal),
	kBufferAlias = MAKE_RESOURCE_CLASS_VAL(ResourceType::kBuffer, ResourceState::kAlias),
	kLastFrameBuffer = MAKE_RESOURCE_CLASS_VAL(ResourceType::kBuffer, ResourceState::kLastFrame)
};
inline constexpr ResourceClass MakeResourceClass(ResourceType type, ResourceState state) {
	return static_cast<ResourceClass>(MAKE_RESOURCE_CLASS_VAL(type, state));
}
inline constexpr ResourceType GetResourceType(ResourceClass res_class) {
	return static_cast<ResourceType>(uint8_t(static_cast<uint8_t>(res_class) & 1u));
}
inline constexpr ResourceState GetResourceState(ResourceClass res_class) {
	return static_cast<ResourceState>(uint8_t(static_cast<uint8_t>(res_class) >> 1u));
}
#undef MAKE_RESOURCE_CLASS_VAL

class ManagedImage;
class ExternalImageBase;
class CombinedImage;
class ImageAlias;
class LastFrameImage;

class ManagedBuffer;
class ExternalBufferBase;
class BufferAlias;
class LastFrameBuffer;

namespace _details_resource_trait_ {
template <typename T> struct ResourceTrait;
template <> struct ResourceTrait<ManagedImage> {
	static constexpr ResourceType kType = ResourceType::kImage;
	static constexpr ResourceState kState = ResourceState::kManaged;
};
template <> struct ResourceTrait<ExternalImageBase> {
	static constexpr ResourceType kType = ResourceType::kImage;
	static constexpr ResourceState kState = ResourceState::kExternal;
};
template <> struct ResourceTrait<CombinedImage> {
	static constexpr ResourceType kType = ResourceType::kImage;
	static constexpr ResourceState kState = ResourceState::kCombinedImage;
};
template <> struct ResourceTrait<ImageAlias> {
	static constexpr ResourceType kType = ResourceType::kImage;
	static constexpr ResourceState kState = ResourceState::kAlias;
};
template <> struct ResourceTrait<LastFrameImage> {
	static constexpr ResourceType kType = ResourceType::kImage;
	static constexpr ResourceState kState = ResourceState::kLastFrame;
};

template <> struct ResourceTrait<ManagedBuffer> {
	static constexpr ResourceType kType = ResourceType::kBuffer;
	static constexpr ResourceState kState = ResourceState::kManaged;
};
template <> struct ResourceTrait<ExternalBufferBase> {
	static constexpr ResourceType kType = ResourceType::kBuffer;
	static constexpr ResourceState kState = ResourceState::kExternal;
};
template <> struct ResourceTrait<BufferAlias> {
	static constexpr ResourceType kType = ResourceType::kBuffer;
	static constexpr ResourceState kState = ResourceState::kAlias;
};
template <> struct ResourceTrait<LastFrameBuffer> {
	static constexpr ResourceType kType = ResourceType::kBuffer;
	static constexpr ResourceState kState = ResourceState::kLastFrame;
};
} // namespace _details_resource_trait_

// Used in Visit function
template <typename RawType> class ResourceVisitorTrait {
private:
	using Type = std::decay_t<std::remove_pointer_t<std::decay_t<RawType>>>;

public:
	static constexpr ResourceType kType = _details_resource_trait_::ResourceTrait<Type>::kType;
	static constexpr ResourceState kState = _details_resource_trait_::ResourceTrait<Type>::kState;
	static constexpr ResourceClass kClass = MakeResourceClass(kType, kState);
	static constexpr bool kIsCombinedOrManagedImage =
	    kClass == ResourceClass::kCombinedImage || kClass == ResourceClass::kManagedImage;
	static constexpr bool kIsCombinedImageChild =

	    kClass == ResourceClass::kImageAlias;
	static constexpr bool kIsAlias = kState == ResourceState::kAlias;
	static constexpr bool kIsLastFrame = kState == ResourceState::kLastFrame;
	static constexpr bool kIsInternal = kState == ResourceState::kManaged || kState == ResourceState::kCombinedImage;
	static constexpr bool kIsExternal = kState == ResourceState::kExternal;
};

class ResourceBase : public ObjectBase {
private:
	ResourceClass m_class{};
	// PassBase *m_producer_pass_ptr{};

	// inline void set_producer_pass_ptr(PassBase *producer_pass_ptr) { m_producer_pass_ptr = producer_pass_ptr; }

	template <typename, typename...> friend class Pool;
	template <typename> friend class AliasOutputPool;
	template <typename> friend class InputPool;
	friend class CombinedImage;

public:
	inline ~ResourceBase() override = default;
	inline ResourceBase(ResourceClass resource_class) : m_class{resource_class} {}
	inline ResourceBase(ResourceBase &&) noexcept = default;

	inline ResourceType GetType() const { return GetResourceType(m_class); }
	inline ResourceState GetState() const { return GetResourceState(m_class); }
	inline ResourceClass GetClass() const { return m_class; }

	template <typename Visitor> std::invoke_result_t<Visitor, ManagedImage *> Visit(Visitor &&visitor);
	template <typename Visitor> std::invoke_result_t<Visitor, ManagedImage *> Visit(Visitor &&visitor) const;
	// virtual bool IsPerFrame() const = 0;
	// virtual void Resize(uint32_t width, uint32_t height) {}

	// inline PassBase *GetProducerPassPtr() const { return m_producer_pass_ptr; }
};

class ImageBase;
class BufferBase;
template <typename RawType> class ResourceTrait {
private:
	using Type = std::decay_t<std::remove_pointer_t<std::decay_t<RawType>>>;
	template <typename Base>
	static constexpr bool kIsBaseOf = std::is_same_v<Base, Type> || std::is_base_of_v<Base, Type>;

public:
	static constexpr bool kIsResource = kIsBaseOf<ResourceBase>;
	static constexpr bool kIsImage = kIsBaseOf<ImageBase>;
	static constexpr bool kIsBuffer = kIsBaseOf<BufferBase>;
};

} // namespace myvk_rg::_details_

#endif

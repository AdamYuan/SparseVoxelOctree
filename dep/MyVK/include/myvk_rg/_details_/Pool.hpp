#ifndef MYVK_RG_POOL_HPP
#define MYVK_RG_POOL_HPP

#include "Macro.hpp"
#include "ObjectBase.hpp"

#include <cinttypes>
#include <cstdio>
#include <limits>
#include <memory>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <variant>

namespace myvk_rg::_details_ {

// Pool Key
class PoolKey {
public:
	using LengthType = uint8_t;
	using IDType = uint16_t;
	inline constexpr static const std::size_t kMaxStrLen = 32 - sizeof(LengthType) - sizeof(IDType);

private:
	union {
		struct {
			IDType m_id;
			LengthType m_len;
			char m_str[kMaxStrLen];
		};
		std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> _32_;
	};
	static_assert(sizeof(_32_) == 32);

public:
	inline PoolKey() : _32_{} {}
	template <typename IntType = IDType, typename = std::enable_if_t<std::is_integral_v<IntType>>>
	inline PoolKey(std::string_view str, IntType id) : m_str{}, m_len(std::min(str.length(), kMaxStrLen)), m_id(id) {
		std::copy(str.begin(), str.begin() + m_len, m_str);
	}
	inline PoolKey(std::string_view str)
	    : m_str{}, m_len(std::min(str.length(), kMaxStrLen)), m_id{std::numeric_limits<IDType>::max()} {
		std::copy(str.begin(), str.begin() + m_len, m_str);
	}
	inline PoolKey(const PoolKey &r) : _32_{r._32_} {}
	inline PoolKey &operator=(const PoolKey &r) {
		_32_ = r._32_;
		return *this;
	}
	inline std::string_view GetName() const { return std::string_view{m_str, m_len}; }
	inline IDType GetID() const { return m_id; }
	inline void SetName(std::string_view str) {
		m_len = std::min(str.length(), kMaxStrLen);
		std::copy(str.begin(), str.begin() + m_len, m_str);
		std::fill(m_str + m_len, m_str + kMaxStrLen, '\0');
	}
	inline void SetID(IDType id) { m_id = id; }

	inline bool operator<(const PoolKey &r) const { return _32_ < r._32_; }
	inline bool operator>(const PoolKey &r) const { return _32_ > r._32_; }
	inline bool operator==(const PoolKey &r) const { return _32_ == r._32_; }
	inline bool operator!=(const PoolKey &r) const { return _32_ != r._32_; }
	struct Hash {
		inline std::size_t operator()(PoolKey const &r) const noexcept {
			return std::get<0>(r._32_) ^ std::get<1>(r._32_) ^ std::get<2>(r._32_) ^ std::get<3>(r._32_);
			// return ((std::get<0>(r._32_) * 37 + std::get<1>(r._32_)) * 37 + std::get<2>(r._32_)) * 37 +
			//        std::get<3>(r._32_);
		}
	};
};
static_assert(sizeof(PoolKey) == 32 && std::is_move_constructible_v<PoolKey>);

// Pool
namespace _details_rg_pool_ {
template <typename Value> using PoolKeyMap = std::unordered_map<PoolKey, Value, PoolKey::Hash>;

template <typename Type> class TypeTraits {
private:
	inline constexpr static bool kIsObject = std::is_base_of_v<ObjectBase, Type>;
	inline constexpr static bool kAlterPtr =
	    (kIsObject && !std::is_final_v<Type>) || !std::is_move_constructible_v<Type>;
	inline constexpr static bool kAlterOptional = !kAlterPtr && (kIsObject || !std::is_default_constructible_v<Type>);

public:
	using AlterType = std::conditional_t<kAlterPtr, std::unique_ptr<Type>,
	                                     std::conditional_t<kAlterOptional, std::optional<Type>, Type>>;
	using VariantAlterType = std::conditional_t<kAlterPtr, std::unique_ptr<Type>, Type>;

	template <typename TypeToCons>
	inline constexpr static bool kCanConstruct =
	    kAlterPtr ? (std::is_base_of_v<Type, TypeToCons> || std::is_same_v<Type, TypeToCons>)
	              : std::is_same_v<Type, TypeToCons>;
	template <typename TypeToGet>
	inline constexpr static bool kCanGet =
	    kAlterPtr ? (std::is_base_of_v<Type, TypeToGet> || std::is_base_of_v<TypeToGet, Type> ||
	                 std::is_same_v<Type, TypeToGet>)
	              : (std::is_base_of_v<TypeToGet, Type> || std::is_same_v<Type, TypeToGet>);

	inline constexpr static bool kCanReset = kAlterPtr || kAlterOptional;

	template <typename TypeToCons, typename... Args, typename = std::enable_if_t<kCanConstruct<TypeToCons>>>
	inline static TypeToCons *Initialize(AlterType &val, Args &&...args) {
		if constexpr (kAlterPtr) {
			auto uptr = std::make_unique<TypeToCons>(std::forward<Args>(args)...);
			TypeToCons *ret = uptr.get();
			val = std::move(uptr);
			return ret;
		} else if constexpr (kAlterOptional) {
			val.emplace(std::forward<Args>(args)...);
			return &(val.value());
		} else {
			val = TypeToCons(std::forward<Args>(args)...);
			return &val;
		}
	}
	inline static void Reset(AlterType &val) {
		static_assert(kCanReset);
		val.reset();
	}
	inline static bool IsInitialized(const AlterType &val) {
		static_assert(kCanReset);
		if constexpr (kAlterPtr)
			return val;
		else if constexpr (kAlterOptional)
			return val.has_value();
		return true;
	}
	template <typename TypeToGet, typename = std::enable_if_t<kCanGet<TypeToGet>>>
	inline static TypeToGet *Get(const AlterType &val) {
		if constexpr (kAlterPtr) {
			if constexpr (std::is_same_v<Type, TypeToGet>)
				return (TypeToGet *)(val.get());
			else
				return (TypeToGet *)dynamic_cast<const TypeToGet *>(val.get());
		} else if constexpr (kAlterOptional) {
			if constexpr (std::is_same_v<Type, TypeToGet>)
				return val.has_value() ? (TypeToGet *)(&(val.value())) : nullptr;
			else
				return val.has_value() ? (TypeToGet *)dynamic_cast<const TypeToGet *>(&(val.value())) : nullptr;
		} else {
			if constexpr (std::is_same_v<Type, TypeToGet>)
				return (TypeToGet *)(&val);
			else
				return (TypeToGet *)dynamic_cast<const TypeToGet *>(&val);
		}
	}
};

template <class T> struct is_unique_ptr_impl : std::false_type {};
template <class T, class D> struct is_unique_ptr_impl<std::unique_ptr<T, D>> : std::true_type {};
template <class T> inline constexpr bool kIsUniquePtr = is_unique_ptr_impl<std::decay_t<T>>::value;

template <typename VariantType, typename TypeToCons, bool UniquePtr, size_t I = 0>
inline constexpr size_t GetVariantConstructIndex() {
	if constexpr (I >= std::variant_size_v<VariantType>) {
		return -1;
	} else {
		using VTI = std::variant_alternative_t<I, VariantType>;
		if constexpr (UniquePtr) {
			if constexpr (std::is_constructible_v<VTI, std::unique_ptr<TypeToCons> &&>)
				return I;
			else
				return (GetVariantConstructIndex<VariantType, TypeToCons, UniquePtr, I + 1>());
		} else {
			if constexpr (std::is_same_v<VTI, TypeToCons>)
				return I;
			else
				return (GetVariantConstructIndex<VariantType, TypeToCons, UniquePtr, I + 1>());
		}
	}
}
template <typename VariantType, typename TypeToCons, bool UniquePtr>
inline constexpr bool kVariantCanConstruct = GetVariantConstructIndex<VariantType, TypeToCons, UniquePtr>() != -1;

template <typename VariantType, typename TypeToGet, size_t I = 0> inline constexpr bool VariantCanGet() {
	if constexpr (I >= std::variant_size_v<VariantType>)
		return false;
	else {
		using VTI = std::variant_alternative_t<I, VariantType>;
		if constexpr (kIsUniquePtr<VTI>) {
			if constexpr (std::is_same_v<TypeToGet, typename std::pointer_traits<VTI>::element_type> ||
			              std::is_base_of_v<TypeToGet, typename std::pointer_traits<VTI>::element_type> ||
			              std::is_base_of_v<typename std::pointer_traits<VTI>::element_type, TypeToGet>)
				return true;
		} else {
			if constexpr (std::is_same_v<TypeToGet, VTI> || std::is_base_of_v<TypeToGet, VTI>)
				return true;
		}
		return VariantCanGet<VariantType, TypeToGet, I + 1>();
	}
}
template <typename TypeToGet> struct VariantGetter {
	template <typename ValType> inline TypeToGet *operator()(const ValType &val) {
		if constexpr (kIsUniquePtr<ValType>) {
			if constexpr (std::is_same_v<TypeToGet, typename std::pointer_traits<ValType>::element_type>)
				return (TypeToGet *)(val.get());
			else if constexpr (std::is_base_of_v<TypeToGet, typename std::pointer_traits<ValType>::element_type> ||
			                   std::is_base_of_v<typename std::pointer_traits<ValType>::element_type, TypeToGet>)
				return (TypeToGet *)dynamic_cast<const TypeToGet *>(val.get());
		} else {
			if constexpr (std::is_same_v<TypeToGet, ValType>)
				return (TypeToGet *)(&val);
			else if constexpr (std::is_base_of_v<TypeToGet, ValType>)
				return (TypeToGet *)dynamic_cast<const TypeToGet *>(&val);
		}
		return nullptr;
	}
};

template <typename... VariantArgs> struct TypeTraits<std::variant<VariantArgs...>> {
private:
	using Type = std::variant<VariantArgs...>;

public:
	using AlterType = Type;
	template <typename TypeToCons>
	inline constexpr static bool kCanConstruct =
	    kVariantCanConstruct<Type, TypeToCons, false> || kVariantCanConstruct<Type, TypeToCons, true>;
	template <typename TypeToGet> inline constexpr static bool kCanGet = VariantCanGet<Type, TypeToGet>();
	inline constexpr static bool kCanReset = true;

	template <typename TypeToCons, typename... Args, typename = std::enable_if_t<kCanConstruct<TypeToCons>>>
	inline static TypeToCons *Initialize(AlterType &val, Args &&...args) {
		if constexpr (kVariantCanConstruct<Type, TypeToCons, false>) {
			// If Don't need Pointer, prefer plain type
			constexpr size_t kIndex = GetVariantConstructIndex<Type, TypeToCons, false>();
			// printf("index = %lu\n", kIndex);
			val.template emplace<kIndex>(TypeToCons(std::forward<Args>(args)...));
			return &(std::get<kIndex>(val));
		} else {
			constexpr size_t kPtrIndex = GetVariantConstructIndex<Type, TypeToCons, true>();
			// printf("ptr_index = %lu\n", kPtrIndex);
			auto uptr = std::make_unique<TypeToCons>(std::forward<Args>(args)...);
			TypeToCons *ret = uptr.get();
			val.template emplace<kPtrIndex>(std::move(uptr));
			return ret;
		}
	}
	inline static void Reset(AlterType &val) { val = std::monostate{}; }
	inline static bool IsInitialized(const AlterType &val) { return val.index(); }
	template <typename TypeToGet, typename = std::enable_if_t<kCanGet<TypeToGet>>>
	inline static TypeToGet *Get(const AlterType &val) {
		return std::visit(VariantGetter<TypeToGet>{}, val);
	}
};
template <typename... Types> struct TypeTuple;
template <typename Type, typename... Others> struct TypeTuple<Type, Others...> {
	using T = decltype(std::tuple_cat(std::declval<std::tuple<typename TypeTraits<Type>::AlterType>>(),
	                                  std::declval<typename TypeTuple<Others...>::T>()));
};
template <> struct TypeTuple<> {
	using T = std::tuple<>;
};
template <typename, typename> struct VariantCat;
template <typename Type, typename... VariantTypes> struct VariantCat<Type, std::variant<VariantTypes...>> {
	using T = std::variant<VariantTypes..., Type>;
};
template <typename Type> struct VariantCat<Type, std::monostate> {
	using T = std::variant<std::monostate, Type>;
};
template <typename...> struct TypeVariant;
template <typename Type, typename... Others> struct TypeVariant<Type, Others...> {
	using T = typename VariantCat<typename TypeTraits<Type>::VariantAlterType, typename TypeVariant<Others...>::T>::T;
};
template <> struct TypeVariant<> {
	using T = std::monostate;
};

// Used for internal processing
template <typename... Types> struct PoolData {
	using TypeTuple = typename _details_rg_pool_::TypeTuple<Types...>::T;
	template <std::size_t Index> using GetRawType = std::tuple_element_t<Index, std::tuple<Types...>>;
	template <std::size_t Index> using GetAlterType = std::tuple_element_t<Index, TypeTuple>;
	template <std::size_t Index, typename T>
	inline static constexpr bool kCanConstruct =
	    _details_rg_pool_::TypeTraits<GetRawType<Index>>::template kCanConstruct<T>;
	template <std::size_t Index, typename T>
	inline static constexpr bool kCanGet = _details_rg_pool_::TypeTraits<GetRawType<Index>>::template kCanGet<T>;
	template <std::size_t Index>
	inline static constexpr bool kCanReset = _details_rg_pool_::TypeTraits<GetRawType<Index>>::kCanReset;

	_details_rg_pool_::PoolKeyMap<TypeTuple> pool;

	template <std::size_t Index, typename TypeToCons, typename... Args, typename MapIterator,
	          typename = std::enable_if_t<kCanConstruct<Index, TypeToCons>>>
	inline TypeToCons *ValueInitialize(const MapIterator &it, Args &&...args) {
		GetAlterType<Index> &ref = std::get<Index>(it->second);
		using RawType = GetRawType<Index>;
		return _details_rg_pool_::TypeTraits<RawType>::template Initialize<TypeToCons>(ref,
		                                                                               std::forward<Args>(args)...);
	}

	template <std::size_t Index, typename MapIterator, typename = std::enable_if_t<kCanReset<Index>>>
	inline bool ValueIsInitialized(const MapIterator &it) const {
		const GetAlterType<Index> &ref = std::get<Index>(it->second);
		using RawType = GetRawType<Index>;
		return _details_rg_pool_::TypeTraits<RawType>::IsInitialized(ref);
	}

	template <std::size_t Index, typename MapIterator, typename = std::enable_if_t<kCanReset<Index>>>
	inline void ValueReset(MapIterator &it) {
		GetAlterType<Index> &ref = std::get<Index>(it->second);
		using RawType = GetRawType<Index>;
		_details_rg_pool_::TypeTraits<RawType>::Reset(ref);
	}

	template <std::size_t Index, typename TypeToGet, typename MapIterator,
	          typename = std::enable_if_t<kCanGet<Index, TypeToGet>>>
	inline TypeToGet *ValueGet(const MapIterator &it) const {
		const GetAlterType<Index> &ref = std::get<Index>(it->second);
		using RawType = GetRawType<Index>;
		return _details_rg_pool_::TypeTraits<RawType>::template Get<TypeToGet>(ref);
	}

	inline PoolData() {
		static_assert(std::is_default_constructible_v<TypeTuple> && std::is_move_constructible_v<TypeTuple>);
		// printf("%s\n", typeid(TypeTuple).name());
	}
	inline PoolData(PoolData &&) noexcept = default;
	inline ~PoolData() = default;
};

} // namespace _details_rg_pool_
template <typename... Types> using PoolVariant = typename _details_rg_pool_::TypeVariant<Types...>::T;

template <typename Derived, typename... Types> class Pool {
private:
	using PoolData = _details_rg_pool_::PoolData<Types...>;
	_details_rg_pool_::PoolData<Types...> m_data;

	template <std::size_t Index, typename TypeToCons, typename... Args, typename MapIterator>
	inline TypeToCons *initialize_and_set_rg_data(const MapIterator &it, Args &&...args) {
		// Initialize ObjectBase
		if constexpr (std::is_base_of_v<ObjectBase, TypeToCons>) {
			static_assert(std::is_base_of_v<RenderGraphBase, Derived> || std::is_base_of_v<ObjectBase, Derived>);
			TypeToCons *ptr = m_data.template ValueInitialize<Index, TypeToCons>(it);
			if (!ptr)
				return nullptr;

			auto base_ptr = static_cast<ObjectBase *>(ptr);
			base_ptr->set_key_ptr(&(it->first));
			if constexpr (std::is_base_of_v<RenderGraphBase, Derived>)
				base_ptr->set_render_graph_ptr((RenderGraphBase *)static_cast<const Derived *>(this));
			else
				base_ptr->set_render_graph_ptr(((ObjectBase *)static_cast<const Derived *>(this))->GetRenderGraphPtr());
			// Initialize ResourceBase
			/* if constexpr (std::is_base_of_v<ResourceBase, TypeToCons>) {
			    auto resource_ptr = static_cast<ResourceBase *>(ptr);
			    if constexpr (std::is_base_of_v<PassBase, Derived>)
			        resource_ptr->set_producer_pass_ptr((PassBase *)static_cast<const Derived *>(this));
			} */
			ptr->Initialize(std::forward<Args>(args)...);
			return ptr;
		} else {
			return m_data.template ValueInitialize<Index, TypeToCons>(it, std::forward<Args>(args)...);
		}
	}

public:
	inline Pool() = default;
	inline Pool(Pool &&) noexcept = default;
	inline virtual ~Pool() = default;

protected:
	// Get PoolData Pointer
	// inline PoolData &GetPoolData() { return m_data; }
	inline const PoolData &GetPoolData() const { return m_data; }
	// Create Tag and Initialize the Main Object
	template <std::size_t Index, typename TypeToCons, typename... Args>
	inline TypeToCons *CreateAndInitialize(const PoolKey &key, Args &&...args) {
		if (m_data.pool.find(key) != m_data.pool.end())
			return nullptr;
		auto it = m_data.pool.insert({key, typename PoolData::TypeTuple{}}).first;
		return initialize_and_set_rg_data<Index, TypeToCons, Args...>(it, std::forward<Args>(args)...);
	}
	template <std::size_t Index, typename TypeToCons, typename... Args>
	inline TypeToCons *CreateAndInitializeForce(const PoolKey &key, Args &&...args) {
		auto it = m_data.pool.find(key);
		if (it == m_data.pool.end())
			it = m_data.pool.insert({key, typename PoolData::TypeTuple{}}).first;
		return initialize_and_set_rg_data<Index, TypeToCons, Args...>(it, std::forward<Args>(args)...);
	}
	// Create Tag Only
	inline void Create(const PoolKey &key) {
		if (m_data.pool.find(key) != m_data.pool.end())
			return;
		m_data.pool.insert(std::make_pair(key, typename PoolData::TypeTuple{}));
	}
	// Initialize Object of a Tag
	template <std::size_t Index, typename TypeToCons, typename... Args>
	inline TypeToCons *Initialize(const PoolKey &key, Args &&...args) {
		auto it = m_data.pool.find(key);
		return it == m_data.pool.end()
		           ? nullptr
		           : initialize_and_set_rg_data<Index, TypeToCons, Args...>(it, std::forward<Args>(args)...);
	}
	// Check whether an Object of a Tag is Initialized
	template <std::size_t Index> inline bool IsInitialized(const PoolKey &key) const {
		auto it = m_data.pool.find(key);
		return it != m_data.pool.end() && m_data.template ValueIsInitialized<Index>(it);
	}
	// Check whether a Tag exists
	inline bool Exist(const PoolKey &key) const { return m_data.pool.find(key) != m_data.pool.end(); }
	// Reset an Object of a Tag
	template <std::size_t Index> inline void Reset(const PoolKey &key) {
		auto it = m_data.pool.find(key);
		if (it != m_data.pool.end())
			m_data.template ValueReset<Index>(it);
	}
	// Get an Object from a Tag, if not Initialized, Initialize it.
	template <std::size_t Index, typename TypeToCons, typename... Args>
	inline TypeToCons *InitializeOrGet(const PoolKey &key, Args &&...args) {
		auto it = m_data.pool.find(key);
		if (it == m_data.pool.end())
			return nullptr;
		return m_data.template ValueIsInitialized<Index>(it)
		           ? (TypeToCons *)m_data.template ValueGet<Index, TypeToCons>(it)
		           : initialize_and_set_rg_data<Index, TypeToCons, Args...>(it, std::forward<Args>(args)...);
	}
	// Delete a Tag and its Objects
	inline void Delete(const PoolKey &key) { m_data.pool.erase(key); }
	// Get an Object from a Tag
	template <std::size_t Index, typename Type> inline Type *Get(const PoolKey &key) const {
		auto it = m_data.pool.find(key);
		return it == m_data.pool.end() ? nullptr : (Type *)m_data.template ValueGet<Index, Type>(it);
	}
	// Delete all Tags and Objects
	inline void Clear() { m_data.pool.clear(); }
};

} // namespace myvk_rg::_details_

#endif

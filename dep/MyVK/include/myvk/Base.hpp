#ifndef MYVK_BASE_HPP
#define MYVK_BASE_HPP

#include "Ptr.hpp"

namespace myvk {
class Base : public std::enable_shared_from_this<Base> {
protected:
	template <typename T> inline Ptr<const T> GetSelfPtr() const {
		return std::dynamic_pointer_cast<const T>(shared_from_this());
	}
	template <typename T> inline Ptr<T> GetSelfPtr() { return std::dynamic_pointer_cast<T>(shared_from_this()); }

public:
	Base() = default;
	virtual ~Base() = default;
	Base(const Base &) = delete;
	Base(Base &&) = delete;
	Base &operator=(const Base &) = delete;
	Base &operator=(Base &&) = delete;
};
} // namespace myvk

#endif

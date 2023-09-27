#ifndef MYVK_PTR_HPP
#define MYVK_PTR_HPP

#include <memory>
#include <type_traits>

namespace myvk {
template <typename T> using Ptr = std::shared_ptr<T>;
}

#endif // MYVK_PTR_HPP

#ifndef SOBOL_SPIRV_HPP
#define SOBOL_SPIRV_HPP

#include <cinttypes>
constexpr uint32_t kSobolCompSpv[] = {
#include "spirv/sobol.comp.u32"
};
#endif

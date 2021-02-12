#ifndef SRC_QUADSPIRV_HPP
#define SRC_QUADSPIRV_HPP

#include <cinttypes>
constexpr uint32_t kQuadVertSpv[] = {
#include "spirv/quad.vert.u32"
};
constexpr uint32_t kQuadSamplerFragSpv[] = {
#include "spirv/quad_sampler.frag.u32"
};
#endif

#ifndef SRC_OCTREETRACERSPIRV_HPP
#define SRC_OCTREETRACERSPIRV_HPP

#include <cinttypes>
constexpr uint32_t kOctreeTracerFragSpv[] = {
#include "spirv/octree_tracer.frag.u32"
};
constexpr uint32_t kOctreeTracerBeamFragSpv[] = {
#include "spirv/octree_tracer_beam.frag.u32"
};
#endif

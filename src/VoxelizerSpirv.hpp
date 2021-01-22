#ifndef SRC_VOXELIZERSPIRV_HPP
#define SRC_VOXELIZERSPIRV_HPP

#include <cinttypes>
constexpr uint32_t kVoxelizerVertSpv[] = {
#include "spirv/voxelizer.vert.u32"
};
constexpr uint32_t kVoxelizerGeomSpv[] = {
#include "spirv/voxelizer.geom.u32"
};
constexpr uint32_t kVoxelizerFragSpv[] = {
#include "spirv/voxelizer.frag.u32"
};
#endif

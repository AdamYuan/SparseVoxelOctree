#ifndef SRC_OCTREEBUILDERSPIRV_HPP
#define SRC_OCTREEBUILDERSPIRV_HPP

#include <cinttypes>
constexpr uint32_t kOctreeTagNodeCompSpv[] = {
#include "spirv/octree_tag_node.comp.u32"
};
constexpr uint32_t kOctreeInitNodeCompSpv[] = {
#include "spirv/octree_init_node.comp.u32"
};
constexpr uint32_t kOctreeAllocNodeCompSpv[] = {
#include "spirv/octree_alloc_node.comp.u32"
};
constexpr uint32_t kOctreeModifyArgCompSpv[] = {
#include "spirv/octree_modify_arg.comp.u32"
};
#endif

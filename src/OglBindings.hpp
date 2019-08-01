//
// Created by adamyuan on 19-5-4.
//

#ifndef SPARSEVOXELOCTREE_OGLBINDINGS_HPP
#define SPARSEVOXELOCTREE_OGLBINDINGS_HPP

#include <GL/gl3w.h>

constexpr GLuint kTextureUBO = 0;
constexpr GLuint kCameraUBO = 5;

constexpr GLuint kAtomicCounterBinding = 1;
constexpr GLuint kVoxelFragmentListSSBO = 2;
constexpr GLuint kOctreeSSBO = 3;
constexpr GLuint kBeamSampler2D = 4;

#endif //SPARSEVOXELOCTREE_OGLBINDINGS_HPP

//
// Created by adamyuan on 19-5-10.
//

#ifndef SPARSEVOXELOCTREE_CONFIG_HPP
#define SPARSEVOXELOCTREE_CONFIG_HPP

constexpr int kWidth = 1280, kHeight = 720;
constexpr float kCamNear = 1.0f / 512.0f, kCamFar = 4.0f;
constexpr float kCamAspectRatio = kWidth / (float)kHeight;

constexpr int kOctreeLevelMin = 1;
constexpr int kOctreeLevelMax = 12;
constexpr int kOctreeNodeNumMin = 1000000;
constexpr int kOctreeNodeNumMax = 500000000;
constexpr int kBeamSize = 8; //for beam optimization
constexpr int kBeamWidth = (kWidth + (kBeamSize - 1)) / kBeamSize + 1;
constexpr int kBeamHeight = (kHeight + (kBeamSize - 1)) / kBeamSize + 1;

constexpr int kMaxBounce = 16;

#endif //SPARSEVOXELOCTREE_CONFIG_HPP

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cinttypes>

constexpr const char *kAppName = "Vulkan SVO";
constexpr uint32_t kWidth = 1280, kHeight = 720;
constexpr uint32_t kFrameCount = 3;

constexpr float kCamNear = 1.0f / 512.0f, kCamFar = 4.0f;
constexpr float kCamAspectRatio = kWidth / (float)kHeight;

constexpr uint32_t kFilenameBufSize = 512;

constexpr uint32_t kLogLimit = 256;

constexpr uint32_t kOctreeLevelMin = 1;
constexpr uint32_t kOctreeLevelMax = 12;
constexpr uint32_t kOctreeNodeNumMin = 1000000;
constexpr uint32_t kOctreeNodeNumMax = 500000000;
constexpr uint32_t kBeamSize = 8; // for beam optimization
constexpr uint32_t kBeamWidth = (kWidth + (kBeamSize - 1)) / kBeamSize + 1;
constexpr uint32_t kBeamHeight = (kHeight + (kBeamSize - 1)) / kBeamSize + 1;

constexpr uint32_t kMaxBounce = 16;

#endif

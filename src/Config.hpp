#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cinttypes>

constexpr const char *kAppName = "Vulkan SVO";
constexpr uint32_t kDefaultWidth = 1280, kDefaultHeight = 720;
constexpr uint32_t kMinWidth = 256, kMinHeight = 256;
constexpr uint32_t kMaxWidth = 3840, kMaxHeight = 3840;
constexpr uint32_t kFrameCount = 3;

constexpr float kCamNear = 1.0f / 512.0f, kCamFar = 4.0f;

constexpr uint32_t kFilenameBufSize = 512;

constexpr uint32_t kLogLimit = 256;

constexpr uint32_t kOctreeLevelMin = 1;
constexpr uint32_t kOctreeLevelMax = 12;
constexpr uint32_t kOctreeNodeNumMin = 1000000;
constexpr uint32_t kOctreeNodeNumMax = 500000000;
constexpr uint32_t kBeamSize = 8; // for beam optimization

constexpr uint32_t kMinBounce = 2;
constexpr uint32_t kDefaultBounce = 4;
constexpr uint32_t kMaxBounce = 16;

constexpr float kDefaultConstantColor = 5.0f;
constexpr float kMaxConstantColor = 100.0f;

constexpr uint32_t kPTResultUpdateInterval = 10;

#endif

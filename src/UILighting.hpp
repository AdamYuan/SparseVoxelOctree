#ifndef UI_LIGHTING_HPP
#define UI_LIGHTING_HPP

#include "Lighting.hpp"

namespace UI {
constexpr const char *kLightingLoadEnvMapModal = "Load Environment Map";

void LightingMenuItems(const std::shared_ptr<myvk::CommandPool> &command_pool, const std::shared_ptr<Lighting> &lighting,
                      const char **open_modal);

void LightingLoadEnvMapModal(const std::shared_ptr<myvk::CommandPool> &command_pool,
                             const std::shared_ptr<Lighting> &lighting);
} // namespace UI

#endif

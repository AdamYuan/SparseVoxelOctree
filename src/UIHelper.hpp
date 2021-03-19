#ifndef IMGUI_HELPER_HPP
#define IMGUI_HELPER_HPP

#include <imgui/imgui.h>

namespace UI {
void LoadFontAwesome();
bool Spinner(const char *label, float radius, int thickness, const ImU32 &color);
void StyleCinder(ImGuiStyle *dst = nullptr);
void SetNextWindowCentering();
bool DragAngle(const char *label, float *v_rad, float v_speed = 1.0f, float v_degrees_min = 0.0f,
               float v_degrees_max = 0.0f, const char *format = "%.0f deg",
               ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp); // If v_min >= v_max we have no bound

bool FileSave(const char *label, const char *btn, char *buf, size_t buf_size, const char *title, int filter_num,
              const char *const *filter_patterns);
bool FileOpen(const char *label, const char *btn, char *buf, size_t buf_size, const char *title, int filter_num,
              const char *const *filter_patterns);
} // namespace UI

#endif

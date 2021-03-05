#ifndef UI_PATH_TRACER_HPP
#define UI_PATH_TRACER_HPP

#include "PathTracerThread.hpp"

namespace UI {
constexpr const char *kPathTracerStartModal = "Start PathTracer", *kPathTracerStopModal = "Stop PathTracer",
                     *kPathTracerExportEXRModal = "Export OpenEXR";

void PathTracerStartButton(const std::shared_ptr<PathTracerThread> &path_tracer_thread, const char **open_modal);
void PathTracerControlButtons(const std::shared_ptr<PathTracerThread> &path_tracer_thread, const char **open_modal);

void PathTracerMenuItems(const std::shared_ptr<PathTracerThread> &path_tracer_thread);
void PathTracerRightStatus(const std::shared_ptr<PathTracerThread> &path_tracer_thread);

void PathTracerStartModal(const std::shared_ptr<PathTracerThread> &path_tracer_thread);
void PathTracerStopModal(const std::shared_ptr<PathTracerThread> &path_tracer_thread);
void PathTracerExportEXRModal(const std::shared_ptr<PathTracerThread> &path_tracer_thread);
} // namespace UI

#endif

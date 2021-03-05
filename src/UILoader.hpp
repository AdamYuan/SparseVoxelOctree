#ifndef UI_LOADER_HPP
#define UI_LOADER_HPP

#include "LoaderThread.hpp"

namespace UI {
constexpr const char *kLoaderLoadSceneModal = "Load Scene", *kLoaderLoadingModal = "Loading";

void LoaderLoadButton(const std::shared_ptr<LoaderThread> &loader_thread, const char **open_modal);

void LoaderLoadSceneModal(const std::shared_ptr<LoaderThread> &loader_thread);
void LoaderLoadingModal(const std::shared_ptr<LoaderThread> &loader_thread);
} // namespace UI

#endif

#include "UILoader.hpp"

#include "Config.hpp"
#include "ImGuiUtil.hpp"
#include <font-awesome/IconsFontAwesome5.h>
#include <imgui/imgui.h>

namespace UI {
void LoaderLoadButton(const std::shared_ptr<LoaderThread> &loader_thread, const char **open_modal) {
	if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN)) {
		*open_modal = kLoaderLoadSceneModal;
	}
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::TextUnformatted("Load Scene");
		ImGui::EndTooltip();
	}
}
void LoaderLoadSceneModal(const std::shared_ptr<LoaderThread> &loader_thread) {
	ImGui::SetNextWindowCentering();
	if (ImGui::BeginPopupModal(kLoaderLoadSceneModal, nullptr,
	                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoMove)) {
		static char name_buf[kFilenameBufSize];
		static int octree_leve = 10;

		constexpr const char *kFilter[] = {"*.obj"};

		ImGui::FileOpen("OBJ Filename", "...", name_buf, kFilenameBufSize, "OBJ Filename", 1, kFilter);
		ImGui::DragInt("Octree Level", &octree_leve, 1, kOctreeLevelMin, kOctreeLevelMax);

		float button_width = (ImGui::GetWindowContentRegionWidth() - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

		if (ImGui::Button("Load", {button_width, 0})) {
			loader_thread->Launch(name_buf, octree_leve);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", {button_width, 0}))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}
void LoaderLoadingModal(const std::shared_ptr<LoaderThread> &loader_thread) {
	ImGui::SetNextWindowCentering();
	if (ImGui::BeginPopupModal(kLoaderLoadingModal, nullptr,
	                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoMove)) {
		ImGui::Spinner("##spinner", 12, 6, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
		ImGui::SameLine();
		ImGui::TextUnformatted(loader_thread->GetNotification());

		ImGui::EndPopup();
	}
}
} // namespace UI

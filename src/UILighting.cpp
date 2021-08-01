#include "UILighting.hpp"

#include "Config.hpp"
#include "UIHelper.hpp"

#include <imgui/imgui.h>

namespace UI {
void LightingMenuItem(const std::shared_ptr<myvk::CommandPool> &command_pool, const std::shared_ptr<Lighting> &lighting,
                      const char **open_modal) {
	if (ImGui::BeginMenu("Light")) {
		if (ImGui::MenuItem("Sun Radiance", nullptr, lighting->m_light_type == Lighting::LightTypes::kSunRadiance))
			lighting->m_light_type = Lighting::LightTypes::kSunRadiance;
		ImGui::BeginGroup();
		ImGui::DragFloat3("", &lighting->m_sun_radiance[0], 0.1f, 0.0f, kMaxSunRadiance);
		ImGui::EndGroup();

		if (ImGui::MenuItem("Environment Map", nullptr,
		                    lighting->m_light_type == Lighting::LightTypes::kEnvironmentMap))
			lighting->m_light_type = Lighting::LightTypes::kEnvironmentMap;
		if (ImGui::Button("Load"))
			*open_modal = kLightingLoadEnvMapModal;
		if (!lighting->GetEnvironmentMapPtr()->Empty()) {
			ImGui::SameLine();
			if (ImGui::Button("Reset")) {
				command_pool->GetQueuePtr()->WaitIdle();
				lighting->GetEnvironmentMapPtr()->Reset();
			} else {
				ImGui::SameLine();
				ImGui::Text("(%d x %d)", lighting->GetEnvironmentMapPtr()->GetImageExtent().width,
				            lighting->GetEnvironmentMapPtr()->GetImageExtent().height);
			}
		}

		ImGui::EndMenu();
	}
}
void LightingLoadEnvMapModal(const std::shared_ptr<myvk::CommandPool> &command_pool,
                             const std::shared_ptr<Lighting> &lighting) {
	UI::SetNextWindowCentering();
	if (ImGui::BeginPopupModal(kLightingLoadEnvMapModal, nullptr,
	                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoMove)) {
		static char name_buf[kFilenameBufSize];

		constexpr const char *kFilter[] = {"*.hdr", "*.exr"};

		UI::FileOpen("Env Map Filename", "...", name_buf, kFilenameBufSize, "Environment Map Filename", 2, kFilter);

		float button_width = (ImGui::GetWindowContentRegionWidth() - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

		if (ImGui::Button("Load", {button_width, 0})) {
			command_pool->GetQueuePtr()->WaitIdle();
			lighting->GetEnvironmentMapPtr()->Reset(command_pool, name_buf);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", {button_width, 0}))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}
} // namespace UI

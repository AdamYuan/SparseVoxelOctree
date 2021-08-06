#include "UILighting.hpp"

#include "Config.hpp"
#include "ImGuiUtil.hpp"

#include <imgui/imgui.h>

namespace UI {
void LightingMenuItems(const std::shared_ptr<myvk::CommandPool> &command_pool,
                       const std::shared_ptr<Lighting> &lighting, const char **open_modal) {
	if (ImGui::BeginMenu("Light")) {
		auto type = lighting->m_light_type;
		bool active;

		{
			active = type == Lighting::LightTypes::kConstantColor;
			if (ImGui::RadioButton("Constant Color", active))
				lighting->m_light_type = Lighting::LightTypes::kConstantColor;

			if (!active)
				ImGui::PushDisabled();
			ImGui::DragFloat3("", &lighting->m_sun_radiance[0], 0.1f, 0.0f, kMaxConstantColor);
			if (!active)
				ImGui::PopDisabled();
		}

		ImGui::Separator();

		{
			active = type == Lighting::LightTypes::kEnvironmentMap;
			if (ImGui::RadioButton("Environment Map", active))
				lighting->m_light_type = Lighting::LightTypes::kEnvironmentMap;

			if (!active)
				ImGui::PushDisabled();
			if (ImGui::Button("Load"))
				*open_modal = kLightingLoadEnvMapModal;
			if (!lighting->GetEnvironmentMapPtr()->Empty()) {
				ImGui::SameLine();
				if (ImGui::Button("Unload")) {
					command_pool->GetQueuePtr()->WaitIdle();
					lighting->GetEnvironmentMapPtr()->Reset();
				} else {
					ImGui::SameLine();
					ImGui::Text("(%d x %d)", lighting->GetEnvironmentMapPtr()->GetImageExtent().width,
					            lighting->GetEnvironmentMapPtr()->GetImageExtent().height);
				}
			}
			ImGui::InputFloat("Multiplier", &lighting->GetEnvironmentMapPtr()->m_multiplier);
			if (lighting->GetEnvironmentMapPtr()->m_multiplier < 0.0f)
				lighting->GetEnvironmentMapPtr()->m_multiplier = 0.0f;
			ImGui::DragAngle("Rotation", &lighting->GetEnvironmentMapPtr()->m_rotation, 1, 0, 360);
			if (!active)
				ImGui::PopDisabled();
		}

		ImGui::EndMenu();
	}
}
void LightingLoadEnvMapModal(const std::shared_ptr<myvk::CommandPool> &command_pool,
                             const std::shared_ptr<Lighting> &lighting) {
	ImGui::SetNextWindowCentering();
	if (ImGui::BeginPopupModal(kLightingLoadEnvMapModal, nullptr,
	                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoMove)) {
		static char name_buf[kFilenameBufSize];

		constexpr const char *kFilter[] = {"*.hdr", "*.exr"};

		ImGui::FileOpen("Env Map Filename", "...", name_buf, kFilenameBufSize, "Environment Map Filename", 2, kFilter);

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

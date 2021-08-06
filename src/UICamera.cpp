#include "UICamera.hpp"

#include "ImGuiUtil.hpp"

namespace UI {
void CameraMenuItems(const std::shared_ptr<Camera> &camera) {
	if (ImGui::BeginMenu("Camera")) {
		ImGui::DragAngle("FOV", &camera->m_fov, 1, 10, 179);
		ImGui::InputFloat3("Position", &camera->m_position[0]);
		ImGui::DragAngle("Yaw", &camera->m_yaw, 1, 0, 360);
		ImGui::DragAngle("Pitch", &camera->m_pitch, 1, -90, 90);
		ImGui::Separator();
		ImGui::DragFloat("Speed", &camera->m_speed, 0.005f, 0.005f, 0.2f);
		ImGui::DragFloat("Sensitivity", &camera->m_sensitivity, 0.001f, 0.001f, 0.02f);
		ImGui::EndMenu();
	}
}
} // namespace UI

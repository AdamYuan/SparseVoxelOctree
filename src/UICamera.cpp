#include "UICamera.hpp"

#include "UIHelper.hpp"

namespace UI {
void CameraMenuItems(const std::shared_ptr<Camera> &camera) {
	if (ImGui::BeginMenu("Camera")) {
		UI::DragAngle("FOV", &camera->m_fov, 1, 10, 179);
		ImGui::DragFloat("Speed", &camera->m_speed, 0.005f, 0.005f, 0.2f);
		ImGui::InputFloat3("Position", &camera->m_position[0]);
		UI::DragAngle("Yaw", &camera->m_yaw, 1, 0, 360);
		UI::DragAngle("Pitch", &camera->m_pitch, 1, -90, 90);
		ImGui::EndMenu();
	}
}
} // namespace UI

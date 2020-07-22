#include "Camera.hpp"
#include "Config.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

void Camera::move_forward(float dist, float dir) {
	m_position.x -= glm::sin(m_yaw + dir) * dist;
	m_position.z -= glm::cos(m_yaw + dir) * dist;
}

void Camera::Control(GLFWwindow *window, float delta) {
	glm::dvec2 cur_pos;
	glfwGetCursorPos(window, &cur_pos.x, &cur_pos.y);

	if (!ImGui::GetCurrentContext()->NavWindow
		|| (ImGui::GetCurrentContext()->NavWindow->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus)) {
		float speed = delta * m_speed;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			move_forward(speed, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			move_forward(speed, PIF * 0.5f);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			move_forward(speed, -PIF * 0.5f);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			move_forward(speed, PIF);
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			m_position.y += speed;
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			m_position.y -= speed;


		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
			glfwGetCursorPos(window, &cur_pos.x, &cur_pos.y);
			float offset_x = float(cur_pos.x - m_last_mouse_pos.x) * m_sensitive;
			float offset_y = float(cur_pos.y - m_last_mouse_pos.y) * m_sensitive;

			m_yaw -= offset_x;
			m_pitch -= offset_y;

			m_pitch = glm::clamp(m_pitch, -PIF * 0.5f, PIF * 0.5f);
			m_yaw = glm::mod(m_yaw, PIF * 2);
		}
	}
	m_last_mouse_pos = cur_pos;
}

void Camera::UpdateMatrices() {
	m_buffer.m_view = glm::rotate(glm::identity<glm::mat4>(), -m_pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	m_buffer.m_view = glm::rotate(m_buffer.m_view, -m_yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	m_buffer.m_view = glm::translate(m_buffer.m_view, -m_position);

	m_buffer.m_projection = glm::perspective(m_fov, kCamAspectRatio, kCamNear, kCamFar);
	m_buffer.m_projection[1][1] *= -1;
}

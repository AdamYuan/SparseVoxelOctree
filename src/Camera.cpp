#include "Camera.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

std::shared_ptr<Camera> Camera::Create(const std::shared_ptr<myvk::Device> &device, uint32_t frame_count) {
	std::shared_ptr<Camera> ret = std::make_shared<Camera>();
	ret->m_descriptor_pool =
	    myvk::DescriptorPool::Create(device, frame_count, {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, frame_count}});
	{
		VkDescriptorSetLayoutBinding camera_binding = {};
		camera_binding.binding = 0;
		camera_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		camera_binding.descriptorCount = 1;
		camera_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		ret->m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(device, {camera_binding});
	}

	ret->m_descriptor_sets = myvk::DescriptorSet::CreateMultiple(
	    ret->m_descriptor_pool,
	    std::vector<std::shared_ptr<myvk::DescriptorSetLayout>>(frame_count, ret->m_descriptor_set_layout));
	ret->m_uniform_buffers.resize(frame_count);

	for (uint32_t i = 0; i < frame_count; ++i) {
		ret->m_uniform_buffers[i] = myvk::Buffer::Create(device, sizeof(UniformData), VMA_MEMORY_USAGE_CPU_TO_GPU,
		                                                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		ret->m_descriptor_sets[i]->UpdateUniformBuffer(ret->m_uniform_buffers[i], 0);
	}

	return ret;
}

void Camera::move_forward(float dist, float dir) {
	m_position.x -= glm::sin(m_yaw + dir) * dist;
	m_position.z -= glm::cos(m_yaw + dir) * dist;
}

void Camera::Control(GLFWwindow *window, float delta) {
	glm::dvec2 cur_pos;
	glfwGetCursorPos(window, &cur_pos.x, &cur_pos.y);

	if (!ImGui::GetCurrentContext()->NavWindow ||
	    (ImGui::GetCurrentContext()->NavWindow->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus)) {
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

Camera::UniformData Camera::fetch_uniform_data() const {
	UniformData data = {};
	data.m_projection = glm::perspective(m_fov, m_aspect_ratio, kCamNear, kCamFar);
	data.m_projection[1][1] *= -1;
	data.m_inv_projection = glm::inverse(data.m_projection);
	data.m_inv_view = glm::rotate(glm::identity<glm::mat4>(), -m_pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	data.m_inv_view = glm::rotate(data.m_inv_view, -m_yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	data.m_inv_view = glm::translate(data.m_inv_view, -m_position);
	data.m_inv_view = glm::inverse(data.m_inv_view);
	return data;
}

void Camera::UpdateFrameUniformBuffer(uint32_t current_frame) const {
	m_uniform_buffers[current_frame]->UpdateData(fetch_uniform_data());
}

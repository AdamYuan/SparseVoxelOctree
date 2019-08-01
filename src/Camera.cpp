//
// Created by adamyuan on 2/1/19.
//
#include "Camera.hpp"
#include "Config.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

void Camera::Initialize()
{
	m_ubo.Initialize();
	GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	m_ubo.Storage(sizeof(GLCamera), flags);
	m_ubo_ptr = (GLCamera *) glMapNamedBufferRange(m_ubo.Get(), 0, sizeof(GLCamera), flags);
}

void Camera::Control(GLFWwindow *window, const mygl3::Framerate &fps)
{
	static glm::dvec2 last_mouse_pos;

	float speed = fps.GetDelta() * m_speed;
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		move_forward(speed, 0.0f);
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		move_forward(speed, PIF * 0.5f);
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		move_forward(speed, -PIF * 0.5f);
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		move_forward(speed, PIF);
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		m_position.y += speed;
	if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		m_position.y -= speed;


	glm::dvec2 cur_pos;
	glfwGetCursorPos(window, &cur_pos.x, &cur_pos.y);

	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
	{
		glfwGetCursorPos(window, &cur_pos.x, &cur_pos.y);
		float offset_x = (cur_pos.x - last_mouse_pos.x) * m_sensitive;
		float offset_y = (cur_pos.y - last_mouse_pos.y) * m_sensitive;

		m_yaw -= offset_x;
		m_pitch -= offset_y;
		m_pitch = glm::clamp(m_pitch, -PIF * 0.5f, PIF * 0.5f);
		m_yaw = glm::mod(m_yaw, PIF * 2);
	}

	last_mouse_pos = cur_pos;
}

void Camera::Update()
{
	m_view = glm::rotate(glm::identity<glm::mat4>(), -m_pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	m_view = glm::rotate(m_view, -m_yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	m_view = glm::translate(m_view, -m_position);

	m_projection = glm::perspective(m_fov, kCamAspectRatio, kCamNear, kCamFar);

	//sync
	GLsync sync_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	GLenum wait_return = GL_UNSIGNALED;
	while (wait_return != GL_ALREADY_SIGNALED && wait_return != GL_CONDITION_SATISFIED)
		wait_return = glClientWaitSync(sync_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
	glDeleteSync(sync_fence);

	//set values
	m_ubo_ptr->m_projection = m_projection;
	m_ubo_ptr->m_view = m_view;
	m_ubo_ptr->m_position = glm::vec4(m_position, 1.0f);
}


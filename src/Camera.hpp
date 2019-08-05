//
// Created by adamyuan on 2/1/19.
//

#ifndef ADYPT_CAMERA_HPP
#define ADYPT_CAMERA_HPP

#include <glm/glm.hpp>
#include <mygl3/buffer.hpp>
#include <mygl3/utils/framerate.hpp>

#define PIF 3.14159265358979323846f

struct GLFWwindow;
class Camera
{
public:
	glm::vec3 m_position{};
	glm::mat4 m_view{}, m_projection{};
	float m_yaw{}, m_pitch{};
	float m_sensitive{0.005f}, m_speed{0.0625f}, m_fov{PIF / 3.0f};

private:
	struct GLCamera
	{
		GLint m_width, m_height, m_beam_size, __a;
		glm::mat4 m_projection;
		glm::mat4 m_view;
		glm::vec4 m_position;
	} *m_ubo_ptr;
	mygl3::Buffer m_ubo;
	void move_forward(float dist, float dir) //degrees
	{
		m_position.x -= glm::sin(m_yaw + dir) * dist;
		m_position.z -= glm::cos(m_yaw + dir) * dist;
	}

public:
	void Initialize();
	void Control(GLFWwindow *window, const mygl3::Framerate &fps);
	void Update();
	const mygl3::Buffer &GetBuffer() const { return m_ubo; }
};

#endif //ADYPT_CAMERA_HPP

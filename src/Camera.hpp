#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <cinttypes>
#include <glm/glm.hpp>

#define PIF 3.14159265358979323846f

struct SVkCamera {
	glm::mat4 m_projection;
	glm::mat4 m_view;
};

struct GLFWwindow;
class Camera
{
	public:
		float m_yaw{0.0f}, m_pitch{0.0f};
		float m_sensitive{0.005f}, m_speed{0.0625f}, m_fov{PIF / 3.0f};
		glm::vec3 m_position{0.0f, 0.0f, 0.0f};

	private:
		void move_forward(float dist, float dir);
		glm::dvec2 m_last_mouse_pos{0.0, 0.0};
		SVkCamera m_buffer;

	public:
		void Control(GLFWwindow *window, float delta);
		void UpdateMatrices();
		const SVkCamera &GetBuffer() const { return m_buffer; }
};

#endif

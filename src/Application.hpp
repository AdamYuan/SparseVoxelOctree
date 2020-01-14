//
// Created by adamyuan on 19-5-3.
//

#ifndef SPARSEVOXELOCTREE_APPLICATION_HPP
#define SPARSEVOXELOCTREE_APPLICATION_HPP

#include <mygl3/utils/framerate.hpp>
#include <memory>
#include "ScreenQuad.hpp"
#include "Octree.hpp"
#include "OctreeTracer.hpp"
#include "PathTracer.hpp"

#include <GLFW/glfw3.h>

class Application
{
private:
	GLFWwindow *m_window;
	Camera m_camera;
	ScreenQuad m_quad;
	OctreeTracer m_octree_tracer;
	PathTracer m_pathtracer;
	mygl3::Framerate m_fps;

	std::unique_ptr<Octree> m_octree;

	bool m_pathtracing_flag{false}, m_ui_display_flag{true};

	void ui_main();
	void ui_push_disable();
	void ui_pop_disable();
	void ui_main_menubar();
	void ui_info_overlay();
	void ui_load_scene_modal();
	void ui_export_exr_modal();
	static bool ui_file_open(const char *label, const char *btn, char *buf, size_t buf_size, const char *title, int filter_num,
					  const char *const *filter_patterns);
	static bool ui_file_save(const char *label, const char *btn, char *buf, size_t buf_size, const char *title,
							 int filter_num, const char *const *filter_patterns);
	static void glfw_key_callback(GLFWwindow *window, int key, int, int action, int);
public:
	Application();
	~Application();
	void LoadScene(const char *filename, int octree_level);
	void Run();
};


#endif //SPARSEVOXELOCTREE_APPLICATION_HPP

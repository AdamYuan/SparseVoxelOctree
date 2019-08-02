//
// Created by adamyuan on 19-5-3.
//

#ifndef SPARSEVOXELOCTREE_APPLICATION_HPP
#define SPARSEVOXELOCTREE_APPLICATION_HPP

#include <mygl3/utils/framerate.hpp>
#include "Scene.hpp"
#include "Octree.hpp"
#include "OctreeTracer.hpp"
#include <GLFW/glfw3.h>

class Application
{
private:
	GLFWwindow *m_window;
	Camera m_camera;
	Octree m_octree;
	OctreeTracer m_octree_tracer;
	mygl3::Framerate m_fps;
public:
	Application();
	~Application();
	void Run();
};


#endif //SPARSEVOXELOCTREE_APPLICATION_HPP

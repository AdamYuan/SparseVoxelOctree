//
// Created by adamyuan on 19-5-3.
//

#include <cstdio>
#include "Voxelizer.hpp"
#include "Application.hpp"
#include "Config.hpp"

Application::Application()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	m_window = glfwCreateWindow(kWidth, kHeight, "SparseVoxelOctree", nullptr, nullptr);
	glfwMakeContextCurrent(m_window);
	glfwSetWindowUserPointer(m_window, (void*)this);

	gl3wInit();

	Scene scene;
	//scene.Initialize("/home/adamyuan/Projects/Adypt/models/sibenik/sibenik.obj");
	//scene.Initialize("/home/adamyuan/Projects/Adypt/models/living_room/living_room.obj");
	scene.Initialize("/home/adamyuan/Projects/Adypt/models/San_Miguel/san-miguel-low-poly.obj");
	//scene.Initialize("models/sponza/sponza.obj");
	//scene.Initialize("/home/adamyuan/Projects/Adypt/models/bmw/bmw.obj");
	//scene.Initialize("/home/adamyuan/Projects/Adypt/models/powerplant/powerplant.obj");
	//scene.Initialize("/home/adamyuan/Projects/Adypt/models/vokselia_spawn/vokselia_spawn.obj");
	//scene.Initialize("/home/adamyuan/Projects/Adypt/models/fireplace_room/fireplace_room.obj");
	//scene.Initialize("/home/adamyuan/Projects/Adypt/models/CornellBox/CornellBox-Original.obj");
	//scene.Initialize("/home/adamyuan/Projects/Adypt/models/conference/conference.obj");
	//scene.Initialize("/home/adamyuan/Projects/Adypt/models/breakfast_room/breakfast_room.obj");
	//scene.Initialize("/home/adamyuan/Projects/Adypt/models/salle_de_bain/salle_de_bain.obj");

	m_camera.Initialize();
	m_camera.m_position = glm::vec3(1.5f);

	m_renderer.Initialize();

	Voxelizer voxelizer;
	voxelizer.Initialize();
	voxelizer.Voxelize(scene);

	m_octree.Initialize();
	m_octree.Build(voxelizer);

	m_octree_tracer.Initialize();
}

Application::~Application()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Application::Run()
{
	char title[64];
	while(!glfwWindowShouldClose(m_window))
	{
		m_fps.Update();
		m_camera.Control(m_window, m_fps);
		m_camera.Update();

		sprintf(title, "fps: %f", m_fps.GetFps());
		glfwSetWindowTitle(m_window, title);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_octree_tracer.Render(m_octree, m_camera);

		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}
}

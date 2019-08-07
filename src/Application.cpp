//
// Created by adamyuan on 19-5-3.
//

#include <cstdio>
#include <portable-file-dialogs.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>
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

	m_camera.Initialize();
	m_camera.m_position = glm::vec3(1.5f);
	m_octree_tracer.Initialize();

	//Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiStyle &st = ImGui::GetStyle();
	st.WindowBorderSize = 0.0f;
	st.Alpha = 0.7f;
	st.WindowRounding = 0.0f;
	st.ChildRounding = 0.0f;
	st.FrameRounding = 0.0f;
	st.ScrollbarRounding = 0.0f;
	st.GrabRounding = 0.0f;
	st.TabRounding = 0.0f;

	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 450 core");

	//Initialize portable-file-dialogs
	pfd::settings::verbose(true);
}

void Application::LoadScene(const char *filename, int octree_level)
{
	if(octree_level < kOctreeLevelMin || octree_level > kOctreeLevelMax) return;

	Scene scene;
	if( scene.Initialize(filename) )
	{
		Voxelizer voxelizer;
		voxelizer.Initialize(octree_level);
		voxelizer.Voxelize(scene);

		m_octree.reset( new Octree );
		m_octree->Initialize(octree_level);
		m_octree->Build(voxelizer);
	}
}

Application::~Application()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Application::Run()
{
	while(!glfwWindowShouldClose(m_window))
	{
		m_fps.Update();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ui_main();

		if(m_octree)
		{
			m_camera.Control(m_window, m_fps);
			m_camera.Update();

			m_octree_tracer.Render(*m_octree, m_camera);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}
}

void Application::ui_main()
{
	ui_main_menubar();
	ui_info_overlay();
}

void Application::ui_info_overlay()
{
	ImGui::SetNextWindowPos(ImVec2(10.0f, ImGui::GetIO().DisplaySize.y - 10.0f),
							ImGuiCond_Always, ImVec2(0, 1));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.4f)); // Transparent background
	if (ImGui::Begin("INFO", nullptr,
					 ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize
					 |ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoMove
					 |ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
		ImGui::Text("OpenGL version: %s", glGetString(GL_VERSION));
		ImGui::Text("FPS: %f", m_fps.GetFps());

		if(m_octree)
		{
			ImGui::Text("Octree Level: %d", m_octree->GetLevel());
		}

		ImGui::End();
	}
	ImGui::PopStyleColor();
}

void Application::ui_main_menubar()
{
	bool open_load_scene_popup = false;

	ImGui::BeginMainMenuBar();

	if(ImGui::MenuItem("Load Scene"))
		open_load_scene_popup = true;

	if(ImGui::BeginMenu("View"))
	{
		if(ImGui::MenuItem("Diffuse", nullptr, m_octree_tracer.m_view_type == OctreeTracer::kDiffuse))
			m_octree_tracer.m_view_type = OctreeTracer::kDiffuse;
		if(ImGui::MenuItem("Normal", nullptr, m_octree_tracer.m_view_type == OctreeTracer::kNormal))
			m_octree_tracer.m_view_type = OctreeTracer::kNormal;
		if(ImGui::MenuItem("Iteration", nullptr, m_octree_tracer.m_view_type == OctreeTracer::kIteration))
			m_octree_tracer.m_view_type = OctreeTracer::kIteration;

		ImGui::EndMenu();
	}

	if(ImGui::BeginMenu("Camera"))
	{
		ImGui::SliderAngle("FOV", &m_camera.m_fov, 10, 180);
		ImGui::DragFloat("Speed", &m_camera.m_speed, 0.005f, 0.005f, 0.2f);
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();

	if(open_load_scene_popup)
		ImGui::OpenPopup("Load Scene##1");

	ui_load_scene_modal();
}
bool
Application::ui_file_open(const char *label, const char *btn, char *buf, size_t buf_size, const char *title,
						  const std::vector<std::string> &filters)
{
	bool ret = ImGui::InputText(label, buf, buf_size);
	ImGui::SameLine();

	if(ImGui::Button(btn))
	{
		auto file_dialog = pfd::open_file(title, "", filters, false);
		if(!file_dialog.result().empty()) strcpy(buf, file_dialog.result().front().c_str());
		ret = true;
	}
	return ret;
}

void Application::ui_load_scene_modal()
{
	if (ImGui::BeginPopupModal("Load Scene##1", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		constexpr size_t kFilenameBufSize = 512;
		static char name_buf[kFilenameBufSize];
		static int octree_leve = 10;

		ui_file_open("OBJ Filename", "...##5", name_buf, kFilenameBufSize, "OBJ Filename",
					 {"OBJ File (.obj)", "*.obj", "All Files", "*"});
		ImGui::DragInt("Octree Level", &octree_leve, 1, 2, 12);

		if (ImGui::Button("Load", ImVec2(256, 0)))
		{
			LoadScene(name_buf, octree_leve);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(256, 0)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}


//
// Created by adamyuan on 19-5-3.
//

#include <cstdio>
#include <portable-file-dialogs.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_internal.h>
#include "Scene.hpp"
#include "Voxelizer.hpp"
#include "Application.hpp"
#include "Config.hpp"

constexpr size_t kFilenameBufSize = 512;

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
	glfwSetKeyCallback(m_window, glfw_key_callback);

	gl3wInit();

	m_quad.Initialize();
	m_camera.Initialize();
	m_camera.m_position = glm::vec3(1.5f);
	m_octree_tracer.Initialize();
	m_pathtracer.Initialize();

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

		if(m_ui_display_flag) ui_main();

		if(m_octree)
		{
			if(m_pathtracing_flag)
				m_pathtracer.Render(m_quad);
			else
			{
				m_camera.Control(m_window, m_fps);
				m_camera.Update();

				m_octree_tracer.Render(m_quad, *m_octree, m_camera);
			}
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

void Application::ui_push_disable()
{
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
}

void Application::ui_pop_disable()
{
	ImGui::PopItemFlag();
	ImGui::PopStyleVar();
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
		ImGui::Text("Toggle UI display with [X]");
		ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
		ImGui::Text("OpenGL version: %s", glGetString(GL_VERSION));
		ImGui::Text("FPS: %f", m_fps.GetFps());

		if(m_octree)
			ImGui::Text("Octree Level: %d", m_octree->GetLevel());

		if(m_pathtracing_flag)
			ImGui::Text("SPP: %d", m_pathtracer.GetSPP());

		ImGui::End();
	}
	ImGui::PopStyleColor();
}

void Application::ui_main_menubar()
{
	bool open_load_scene_popup = false, open_export_exr_popup = false;

	ImGui::BeginMainMenuBar();

	if(! m_pathtracing_flag)
	{
		if(ImGui::Button("Load Scene"))
			open_load_scene_popup = true;

		if(ImGui::BeginMenu("Camera"))
		{
			ImGui::SliderAngle("FOV", &m_camera.m_fov, 10, 180);
			ImGui::DragFloat("Speed", &m_camera.m_speed, 0.005f, 0.005f, 0.2f);
			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("Primary View"))
		{
			if(ImGui::MenuItem("Diffuse", nullptr, m_octree_tracer.m_view_type == OctreeTracer::kDiffuse))
				m_octree_tracer.m_view_type = OctreeTracer::kDiffuse;
			if(ImGui::MenuItem("Normal", nullptr, m_octree_tracer.m_view_type == OctreeTracer::kNormal))
				m_octree_tracer.m_view_type = OctreeTracer::kNormal;
			if(ImGui::MenuItem("Iterations", nullptr, m_octree_tracer.m_view_type == OctreeTracer::kIteration))
				m_octree_tracer.m_view_type = OctreeTracer::kIteration;
			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("Beam Optimization"))
		{
			if(ImGui::MenuItem("Enable", nullptr, m_octree_tracer.m_beam_enable))
				m_octree_tracer.m_beam_enable ^= 1;
			ImGui::DragFloat("Ray Direction Size", &m_octree_tracer.m_beam_dir_size, 0.001f, 0.0f, 0.1f);
			ImGui::DragFloat("Ray Origin Size", &m_octree_tracer.m_beam_origin_size, 0.001f, 0.0f, 0.1f);
			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("Path Tracer"))
		{
			ImGui::DragInt("Bounce", &m_pathtracer.m_bounce, 1, 2, kMaxBounce);
			ImGui::DragFloat3("Sun Radiance", m_pathtracer.m_sun_radiance.data.data, 0.1f, 0.0f, 20.0f);
			ImGui::EndMenu();
		}
	}

	if(m_octree)
	{
		if(m_pathtracing_flag && ImGui::Button("Export OpenEXR"))
			open_export_exr_popup = true;
		if(ImGui::Checkbox("Start PT", &m_pathtracing_flag))
		{
			if(m_pathtracing_flag)
				m_pathtracer.Prepare(m_camera, *m_octree, m_octree_tracer);
		}
	}

	ImGui::EndMainMenuBar();

	if(open_load_scene_popup)
		ImGui::OpenPopup("Load Scene");
	if(open_export_exr_popup)
	{
		m_pathtracer.m_pause = true;
		ImGui::OpenPopup("Export OpenEXR");
	}

	ui_load_scene_modal();
	ui_export_exr_modal();
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

bool
Application::ui_file_save(const char *label, const char *btn, char *buf, size_t buf_size, const char *title,
						  const std::vector<std::string> &filters)
{
	bool ret = ImGui::InputText(label, buf, buf_size);
	ImGui::SameLine();

	if(ImGui::Button(btn))
	{
		auto file_dialog = pfd::save_file(title, "", filters, true);
		strcpy(buf, file_dialog.result().c_str());
		ret = true;
	}
	return ret;
}

void Application::ui_load_scene_modal()
{
	if (ImGui::BeginPopupModal("Load Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
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

void Application::ui_export_exr_modal()
{
	if (ImGui::BeginPopupModal("Export OpenEXR", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char exr_name_buf[kFilenameBufSize]{};
		static bool save_as_fp16{false};
		ui_file_save("OpenEXR Filename", "...##0", exr_name_buf, kFilenameBufSize, "Export OpenEXR",
					 {"OpenEXR File (.exr)", "*.exr", "All Files", "*"});

		ImGui::Checkbox("Export As FP16", &save_as_fp16);

		{
			if (ImGui::Button("Export", ImVec2(256, 0)))
			{
				m_pathtracer.Save(exr_name_buf, save_as_fp16);
				m_pathtracer.m_pause = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(256, 0)))
			{
				m_pathtracer.m_pause = false;
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
}

void Application::glfw_key_callback(GLFWwindow *window, int key, int, int action, int)
{
	auto *app = (Application *)glfwGetWindowUserPointer(window);
	if(!ImGui::GetCurrentContext()->NavWindow
	   || (ImGui::GetCurrentContext()->NavWindow->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_X)
			app->m_ui_display_flag ^= 1u;
	}
}

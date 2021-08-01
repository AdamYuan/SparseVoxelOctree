#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <GLFW/glfw3.h>
#include <volk.h>

#include <array>
#include <memory>
#include <vector>

#include <spdlog/sinks/ringbuffer_sink.h>

#include "Camera.hpp"
#include "ImGuiRenderer.hpp"
#include "LoaderThread.hpp"
#include "Octree.hpp"
#include "OctreeTracer.hpp"
#include "PathTracer.hpp"
#include "PathTracerThread.hpp"
#include "PathTracerViewer.hpp"
#include "Scene.hpp"
#include "Lighting.hpp"

#include "myvk/Buffer.hpp"
#include "myvk/CommandBuffer.hpp"
#include "myvk/CommandPool.hpp"
#include "myvk/DescriptorPool.hpp"
#include "myvk/DescriptorSet.hpp"
#include "myvk/Device.hpp"
#include "myvk/FrameManager.hpp"
#include "myvk/Framebuffer.hpp"
#include "myvk/GraphicsPipeline.hpp"
#include "myvk/Image.hpp"
#include "myvk/ImageView.hpp"
#include "myvk/Instance.hpp"
#include "myvk/PipelineLayout.hpp"
#include "myvk/Queue.hpp"
#include "myvk/RenderPass.hpp"
#include "myvk/Sampler.hpp"
#include "myvk/ShaderModule.hpp"
#include "myvk/Surface.hpp"
#include "myvk/Swapchain.hpp"
#include "myvk/SwapchainImage.hpp"

class Application {
private:
	GLFWwindow *m_window{nullptr};

	// base
	std::shared_ptr<myvk::Instance> m_instance;
	std::shared_ptr<myvk::Surface> m_surface;
	std::shared_ptr<myvk::Device> m_device;
	std::shared_ptr<myvk::Queue> m_main_queue, m_loader_queue, m_path_tracer_queue;
	std::shared_ptr<myvk::PresentQueue> m_present_queue;
	std::shared_ptr<myvk::CommandPool> m_main_command_pool, m_path_tracer_command_pool;

	// frame objects
	myvk::FrameManager m_frame_manager;
	std::vector<std::shared_ptr<myvk::Framebuffer>> m_framebuffers;
	std::vector<std::shared_ptr<myvk::CommandPool>> m_frame_command_pools;
	std::vector<std::shared_ptr<myvk::CommandBuffer>> m_frame_command_buffers;

	// render pass
	std::shared_ptr<myvk::RenderPass> m_render_pass;

	ImGuiRenderer m_imgui_renderer;

	// global resources
	std::shared_ptr<Camera> m_camera;
	std::shared_ptr<Octree> m_octree;
	std::shared_ptr<OctreeTracer> m_octree_tracer;
	std::shared_ptr<PathTracer> m_path_tracer;
	std::shared_ptr<PathTracerViewer> m_path_tracer_viewer;
	std::shared_ptr<EnvironmentMap> m_environment_map;
	std::shared_ptr<Lighting> m_lighting;

	// multithreading loader
	std::shared_ptr<LoaderThread> m_loader_thread;
	std::shared_ptr<PathTracerThread> m_path_tracer_thread;

	// ui flags
	enum class UIStates { kEmpty, kOctreeTracer, kPathTracing, kLoading } m_ui_state{UIStates::kEmpty};
	bool m_ui_display_flag{true};

	std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> m_log_sink;

	void create_window();
	void initialize_vulkan();
	void create_render_pass();
	void create_framebuffers();
	void resize();
	void draw_frame();

	void ui_switch_state();
	void ui_render_main();
	void ui_menubar();

	static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
	static void glfw_framebuffer_resize_callback(GLFWwindow *window, int width, int height);

public:
	Application();
	~Application();
	void Load(const char *filename, uint32_t octree_level);
	void Run();
};

#endif

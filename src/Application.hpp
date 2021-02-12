#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <GLFW/glfw3.h>
#include <volk.h>

#include <array>
#include <condition_variable>
#include <memory>
#include <thread>
#include <vector>

#include <spdlog/sinks/ringbuffer_sink.h>

#include "Camera.hpp"
#include "ImGuiRenderer.hpp"
#include "Octree.hpp"
#include "OctreeTracer.hpp"
#include "PathTracer.hpp"
#include "PathTracerViewer.hpp"
#include "Scene.hpp"
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
	std::shared_ptr<myvk::Swapchain> m_swapchain;
	std::vector<std::shared_ptr<myvk::SwapchainImage>> m_swapchain_images;
	std::vector<std::shared_ptr<myvk::ImageView>> m_swapchain_image_views;
	std::vector<std::shared_ptr<myvk::Framebuffer>> m_framebuffers;
	std::vector<std::shared_ptr<myvk::CommandBuffer>> m_frame_command_buffers;
	myvk::FrameManager m_frame_manager;

	// render pass
	std::shared_ptr<myvk::RenderPass> m_render_pass;

	// global data
	Camera m_camera;
	ImGuiRenderer m_imgui_renderer;
	Octree m_octree;
	OctreeTracer m_octree_tracer;
	PathTracer m_path_tracer;
	PathTracerViewer m_path_tracer_viewer;

	// multithreading loader
	std::thread m_loader_thread;
	std::mutex m_loader_mutex;
	bool m_loader_ready_to_join{false};
	std::condition_variable m_loader_condition_variable;

	// multithreading path tracer
	uint32_t m_path_tracer_spp{0};
	double m_path_tracer_start_time{0.0};
	std::thread m_path_tracer_thread;
	std::mutex m_path_tracer_mutex;
	// bool m_path_tracer_ready_to_join{false};
	// std::condition_variable m_path_tracer_condition_variable;

	// ui flags
	enum class UIStates { kEmpty, kOctreeTracer, kPathTracing, kLoading } m_ui_state{UIStates::kEmpty};
	bool m_ui_display_flag{true};

	std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> m_log_sink;

	void create_window();

	void initialize_vulkan();

	void loader_thread(const char *filename, uint32_t octree_level);

	void path_tracer_thread();

	void create_render_pass();

	void create_framebuffers();

	void draw_frame();

	void ui_switch_state();

	void ui_render_main();

	static void ui_push_disable();

	static void ui_pop_disable();

	void ui_menubar();

	void ui_load_scene_modal();

	void ui_export_exr_modal();

	void ui_loading_modal();

	static bool ui_file_open(const char *label, const char *btn, char *buf, size_t buf_size, const char *title,
	                         int filter_num, const char *const *filter_patterns);

	static bool ui_file_save(const char *label, const char *btn, char *buf, size_t buf_size, const char *title,
	                         int filter_num, const char *const *filter_patterns);

	static void glfw_key_callback(GLFWwindow *window, int key, int, int action, int);

public:
	Application();

	void LoadScene(const char *filename, uint32_t octree_level);

	void Run();

	~Application();
};

#endif

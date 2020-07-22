#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <volk.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <array>
#include <vector>

#include "ImGuiRenderer.hpp"
#include "Camera.hpp"
#include "myvk/Queue.hpp"
#include "myvk/Instance.hpp"
#include "myvk/Device.hpp"
#include "myvk/Swapchain.hpp"
#include "myvk/Buffer.hpp"
#include "myvk/Surface.hpp"
#include "myvk/SwapchainImage.hpp"
#include "myvk/ImageView.hpp"
#include "myvk/RenderPass.hpp"
#include "myvk/CommandPool.hpp"
#include "myvk/CommandBuffer.hpp"
#include "myvk/PipelineLayout.hpp"
#include "myvk/ShaderModule.hpp"
#include "myvk/GraphicsPipeline.hpp"
#include "myvk/Framebuffer.hpp"
#include "myvk/DescriptorPool.hpp"
#include "myvk/DescriptorSet.hpp"
#include "myvk//FrameManager.hpp"
#include "myvk/Image.hpp"
#include "myvk/Sampler.hpp"
#include "Scene.hpp"
#include "Octree.hpp"
#include "OctreeTracer.hpp"

class Application {
private:
	GLFWwindow *m_window{nullptr};

	//base
	std::shared_ptr<myvk::Instance> m_instance;
	std::shared_ptr<myvk::Surface> m_surface;
	std::shared_ptr<myvk::Device> m_device;
	std::shared_ptr<myvk::Queue> m_graphics_compute_queue, m_async_compute_queue;
	std::shared_ptr<myvk::PresentQueue> m_present_queue;
	std::shared_ptr<myvk::CommandPool> m_graphics_compute_command_pool;

	//frame objects
	std::shared_ptr<myvk::Swapchain> m_swapchain;
	std::vector<std::shared_ptr<myvk::SwapchainImage>> m_swapchain_images;
	std::vector<std::shared_ptr<myvk::ImageView>> m_swapchain_image_views;
	std::vector<std::shared_ptr<myvk::Framebuffer>> m_framebuffers;
	std::vector<std::shared_ptr<myvk::CommandBuffer>> m_frame_command_buffers;
	myvk::FrameManager m_frame_manager;

	//render pass
	std::shared_ptr<myvk::RenderPass> m_render_pass;

	//global data
	Camera m_camera;
	ImGuiRenderer m_imgui_renderer;
	Octree m_octree;
	OctreeTracer m_octree_tracer;

	//ui flags
	bool m_pathtracing_flag{false}, m_ui_display_flag{true};

	void create_window();

	void initialize_vulkan();

	void create_render_pass();

	void create_framebuffers();

	void draw_frame();

	void ui_main();

	static void ui_push_disable();

	static void ui_pop_disable();

	void ui_main_menubar();

	void ui_info_overlay();

	void ui_load_scene_modal();

	void ui_export_exr_modal();

	static bool
	ui_file_open(const char *label, const char *btn, char *buf, size_t buf_size, const char *title, int filter_num,
				 const char *const *filter_patterns);

	static bool ui_file_save(const char *label, const char *btn, char *buf, size_t buf_size, const char *title,
							 int filter_num, const char *const *filter_patterns);

	static void glfw_key_callback(GLFWwindow *window, int key, int, int action, int);

public:
	Application();

	void LoadScene(const char *filename, uint32_t octree_level);

	void Run();
};

#endif

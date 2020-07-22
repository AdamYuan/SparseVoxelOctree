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
	std::shared_ptr<myvk::DescriptorPool> m_descriptor_pool;

	//frame objects
	std::shared_ptr<myvk::Swapchain> m_swapchain;
	std::vector<std::shared_ptr<myvk::SwapchainImage>> m_swapchain_images;
	std::vector<std::shared_ptr<myvk::ImageView>> m_swapchain_image_views;
	std::vector<std::shared_ptr<myvk::Framebuffer>> m_framebuffers;
	myvk::FrameManager m_frame_manager;

	struct FrameResource {
		std::shared_ptr<myvk::CommandBuffer> m_command_buffer;
		std::shared_ptr<myvk::Buffer> m_uniform_buffer;
		std::shared_ptr<myvk::DescriptorSet> m_descriptor_set;
	};
	std::vector<FrameResource> m_frame_resources;

	//pipeline
	std::shared_ptr<myvk::RenderPass> m_render_pass;
	std::shared_ptr<myvk::DescriptorSetLayout> m_descriptor_set_layout;
	std::shared_ptr<myvk::PipelineLayout> m_pipeline_layout;
	std::shared_ptr<myvk::GraphicsPipeline> m_pipeline;

	//depth buffer
	std::shared_ptr<myvk::Image> m_depth_image;
	std::shared_ptr<myvk::ImageView> m_depth_image_view;

	Camera m_camera;
	Scene m_scene;
	ImGuiRenderer m_imgui_renderer;
	std::shared_ptr<myvk::Buffer> m_octree;

	void create_window();

	void initialize_vulkan();

	void create_depth_buffer();

	void create_render_pass();

	void create_descriptor_set_layout();

	void create_graphics_pipeline();

	void create_framebuffers();

	void create_descriptor_pool();

	void create_frame_resources();

	void draw_frame();

	bool m_pathtracing_flag{false}, m_ui_display_flag{true};
	void ui_main();
	static void ui_push_disable();
	static void ui_pop_disable();
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

	void LoadScene(const char *filename, uint32_t octree_level);

	void Run();
};

#endif

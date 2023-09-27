#include "Application.hpp"

#include "Config.hpp"
#include <spdlog/spdlog.h>

#include <myvk/GLFWHelper.hpp>
#include <myvk/ImGuiHelper.hpp>
#include <myvk/QueueSelector.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include "ImGuiUtil.hpp"
#include "UICamera.hpp"
#include "UILighting.hpp"
#include "UILoader.hpp"
#include "UILog.hpp"
#include "UIOctreeTracer.hpp"
#include "UIPathTracer.hpp"

#ifndef NDEBUG

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                     VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                     void *user_data) {
	if (message_severity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		spdlog::error("{}", callback_data->pMessage);
	else if (message_severity >=
	         VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		spdlog::warn("{}", callback_data->pMessage);
	else
		spdlog::info("{}", callback_data->pMessage);
	return VK_FALSE;
}

#endif

void Application::create_window() {
	m_window = myvk::GLFWCreateWindow(kAppName, kDefaultWidth, kDefaultHeight, true);
	glfwSetWindowUserPointer(m_window, this);
	glfwSetKeyCallback(m_window, glfw_key_callback);
	glfwSetFramebufferSizeCallback(m_window, glfw_framebuffer_resize_callback);
}

void Application::create_render_pass() {
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = m_frame_manager->GetSwapchain()->GetImageFormat();
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::vector<VkSubpassDescription> subpasses(2);
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &color_attachment_ref;
	subpasses[0].pDepthStencilAttachment = nullptr;

	subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[1].colorAttachmentCount = 1;
	subpasses[1].pColorAttachments = &color_attachment_ref;
	subpasses[1].pDepthStencilAttachment = nullptr;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = subpasses.size();
	render_pass_info.pSubpasses = subpasses.data();

	std::vector<VkSubpassDependency> subpass_dependencies(3);
	subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpass_dependencies[0].dstSubpass = 0;
	subpass_dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependencies[0].srcAccessMask = 0;
	subpass_dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	subpass_dependencies[1].srcSubpass = 0;
	subpass_dependencies[1].dstSubpass = 1;
	subpass_dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	subpass_dependencies[2].srcSubpass = 1;
	subpass_dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpass_dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpass_dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpass_dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	render_pass_info.dependencyCount = subpass_dependencies.size();
	render_pass_info.pDependencies = subpass_dependencies.data();

	m_render_pass = myvk::RenderPass::Create(m_device, render_pass_info);
}

void Application::create_framebuffers() {
	m_framebuffers.resize(m_frame_manager->GetSwapchain()->GetImageCount());
	for (uint32_t i = 0; i < m_frame_manager->GetSwapchain()->GetImageCount(); ++i) {
		m_framebuffers[i] = myvk::Framebuffer::Create(m_render_pass, m_frame_manager->GetSwapchainImageViews()[i]);
	}
}

void Application::resize() {
	for (uint32_t i = 0; i < m_frame_manager->GetSwapchain()->GetImageCount(); ++i) {
		m_framebuffers[i] = myvk::Framebuffer::Create(m_render_pass, m_frame_manager->GetSwapchainImageViews()[i]);
	}
	VkExtent2D extent = m_frame_manager->GetSwapchain()->GetExtent();
	m_camera->m_aspect_ratio = extent.width / float(extent.height);
	m_path_tracer_viewer->Resize(extent.width, extent.height);
	m_octree_tracer->Resize(extent.width, extent.height);
}

void Application::draw_frame() {
	if (!m_frame_manager->NewFrame())
		return;

	uint32_t image_index = m_frame_manager->GetCurrentImageIndex();
	uint32_t current_frame = m_frame_manager->GetCurrentFrame();
	if (m_ui_state == UIStates::kOctreeTracer)
		m_camera->UpdateFrameUniformBuffer(current_frame);

	const std::shared_ptr<myvk::CommandBuffer> &command_buffer = m_frame_manager->GetCurrentCommandBuffer();

	command_buffer->GetCommandPoolPtr()->Reset();
	command_buffer->Begin();

	if (m_ui_state != UIStates::kPathTracing && !m_octree->Empty()) {
		m_octree_tracer->CmdBeamRenderPass(command_buffer, current_frame);
	}
	command_buffer->CmdBeginRenderPass(m_render_pass, m_framebuffers[image_index], {{{0.0f, 0.0f, 0.0f, 1.0f}}});
	if (m_ui_state == UIStates::kPathTracing) {
		m_path_tracer_viewer->CmdDrawPipeline(command_buffer, current_frame);
	} else if (!m_octree->Empty()) {
		m_octree_tracer->CmdDrawPipeline(command_buffer, current_frame);
	}
	command_buffer->CmdNextSubpass();
	m_imgui_renderer->CmdDrawPipeline(command_buffer, current_frame);
	command_buffer->CmdEndRenderPass();
	command_buffer->End();

	m_frame_manager->Render();
}

void Application::initialize_vulkan() {
	if (volkInitialize() != VK_SUCCESS) {
		spdlog::error("Failed to load vulkan!");
		exit(EXIT_FAILURE);
	}

#ifdef NDEBUG
	m_instance = myvk::Instance::CreateWithGlfwExtensions();
#else
	m_instance = myvk::Instance::CreateWithGlfwExtensions(true, debug_callback);
#endif
	if (!m_instance) {
		spdlog::error("Failed to create instance!");
		exit(EXIT_FAILURE);
	}

	std::vector<std::shared_ptr<myvk::PhysicalDevice>> physical_devices = myvk::PhysicalDevice::Fetch(m_instance);
	if (physical_devices.empty()) {
		spdlog::error("Failed to find physical device with vulkan support!");
		exit(EXIT_FAILURE);
	}

	m_surface = myvk::Surface::Create(m_instance, m_window);
	if (!m_surface) {
		spdlog::error("Failed to create surface!");
		exit(EXIT_FAILURE);
	}

	// DEVICE CREATION
	{
		const auto &physical_device = physical_devices[0];
		spdlog::info("Physical Device: {}", physical_device->GetProperties().vk10.deviceName);

		std::vector<const char *> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

		if (physical_device->GetExtensionSupport(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME)) {
			extensions.push_back(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
			spdlog::info("EXT_conservative_rasterization supported");
		} else {
			spdlog::warn("EXT_conservative_rasterization not supported");
		}

		const auto queue_selector =
		    [this](const myvk::Ptr<const myvk::PhysicalDevice> &physical_device) -> std::vector<myvk::QueueSelection> {
			const auto &families = physical_device->GetQueueFamilyProperties();
			if (families.empty())
				return std::vector<myvk::QueueSelection>{};

			std::optional<uint32_t> main_queue_family, present_queue_family, loader_queue_family,
			    path_tracer_queue_family;

			// main queue and present queue
			for (uint32_t i = 0; i < families.size(); ++i) {
				VkQueueFlags flags = families[i].queueFlags;
				if ((flags & VK_QUEUE_GRAPHICS_BIT) && (flags & VK_QUEUE_TRANSFER_BIT)) {
					main_queue_family = i;
					// main_queue.index_specifier = 0;

					if (physical_device->GetQueueSurfaceSupport(i, m_surface)) {
						present_queue_family = i;
						// present_queue.index_specifier = 0;
						break;
					}
				}
			}

			// present queue fallback
			if (!present_queue_family.has_value())
				for (uint32_t i = 0; i < families.size(); ++i) {
					if (physical_device->GetQueueSurfaceSupport(i, m_surface)) {
						present_queue_family = i;
						// present_queue.index_specifier = 0;
						break;
					}
				}

			// loader queue
			for (uint32_t i = 0; i < families.size(); ++i) {
				VkQueueFlags flags = families[i].queueFlags;
				if ((flags & VK_QUEUE_GRAPHICS_BIT) && (flags & VK_QUEUE_COMPUTE_BIT) &&
				    (flags & VK_QUEUE_TRANSFER_BIT)) {
					loader_queue_family = i;
					// loader_queue.index_specifier = 1;

					if (i != main_queue_family.value_or(-1))
						break; // prefer independent queue
				}
			}

			// path tracer queue
			for (uint32_t i = 0; i < families.size(); ++i) {
				VkQueueFlags flags = families[i].queueFlags;
				if ((flags & VK_QUEUE_COMPUTE_BIT) && (flags & VK_QUEUE_TRANSFER_BIT)) {
					path_tracer_queue_family = i;
					// path_tracer_queue.index_specifier = 1;

					if (i != main_queue_family.value_or(-1))
						break; // prefer independent queue
				}
			}

			return {
			    myvk::QueueSelection{&m_main_queue, main_queue_family.value(), 0},
			    myvk::QueueSelection{m_surface, &m_present_queue, present_queue_family.value(), 0},
			    myvk::QueueSelection{&m_loader_queue, loader_queue_family.value(), 1},
			    myvk::QueueSelection{&m_path_tracer_queue, path_tracer_queue_family.value(), 1},
			};
		};

		auto features = physical_device->GetDefaultFeatures();
		features.vk12.descriptorBindingPartiallyBound = VK_TRUE;
		m_device = myvk::Device::Create(physical_device, queue_selector, features, extensions);
		if (!m_device) {
			spdlog::error("Failed to create logical device!");
			exit(EXIT_FAILURE);
		}

		spdlog::info("Present Queue: ({}){}, Main Queue: ({}){}, Loader Queue: ({}){}, PathTracer Queue: ({}){}",
		             m_present_queue->GetFamilyIndex(), (void *)m_present_queue->GetHandle(), // present queue
		             m_main_queue->GetFamilyIndex(), (void *)m_main_queue->GetHandle(),       // main queue
		             m_loader_queue->GetFamilyIndex(), (void *)m_loader_queue->GetHandle(),   // loader queue
		             m_path_tracer_queue->GetFamilyIndex(),
		             (void *)m_path_tracer_queue->GetHandle() // path tracer queue
		);

		if (m_path_tracer_queue->GetFamilyIndex() == m_main_queue->GetFamilyIndex()) {
			spdlog::warn("Async path tracing queue not available, the main thread might be blocked when path tracing");
		}
	}

	m_main_command_pool = myvk::CommandPool::Create(m_main_queue);
	m_path_tracer_command_pool = myvk::CommandPool::Create(m_path_tracer_queue);
}

Application::Application() {
	m_log_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(kLogLimit);
	UI::LogSetRequiredPattern(m_log_sink);
	spdlog::default_logger()->sinks().push_back(m_log_sink);

	create_window();
	initialize_vulkan();

	myvk::ImGuiInit(m_window, myvk::CommandPool::Create(m_main_queue), []() {
		ImGui::LoadFontAwesome();
		ImGui::StyleCinder();
	});

	m_frame_manager = myvk::FrameManager::Create(m_main_queue, m_present_queue, false, kFrameCount);
	m_frame_manager->SetResizeFunc([this](const VkExtent2D &) { resize(); });

	glfwSetWindowTitle(
	    m_window,
	    (std::string{kAppName} + " | " + m_device->GetPhysicalDevicePtr()->GetProperties().vk10.deviceName).c_str());
	create_render_pass();
	create_framebuffers();
	m_imgui_renderer = myvk::ImGuiRenderer::Create(m_render_pass, 1, kFrameCount);

	m_environment_map = EnvironmentMap::Create(m_device);
	m_lighting = Lighting::Create(m_environment_map);

	m_camera = Camera::Create(m_device, kFrameCount + 1); // reserve a camera buffer for path tracer
	m_camera->m_position = glm::vec3(1.5);
	m_octree = Octree::Create(m_device);
	m_octree_tracer = OctreeTracer::Create(m_octree, m_camera, m_lighting, m_render_pass, 0, kFrameCount);
	m_path_tracer = PathTracer::Create(m_octree, m_camera, m_lighting, m_path_tracer_command_pool);
	m_path_tracer_viewer = PathTracerViewer::Create(m_path_tracer, m_render_pass, 0);

	m_loader_thread = LoaderThread::Create(m_octree, m_loader_queue, m_main_queue);
	m_path_tracer_thread = PathTracerThread::Create(m_path_tracer_viewer, m_path_tracer_queue, m_main_queue);
}

Application::~Application() {
	// Stop threads
	m_path_tracer_thread = nullptr;
	m_loader_thread = nullptr;
	// Wait all work done
	m_device->WaitIdle();

	ImGui_ImplGlfw_Shutdown();
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Application::Load(const char *filename, uint32_t octree_level) { m_loader_thread->Launch(filename, octree_level); }

void Application::Run() {
	double lst_time = glfwGetTime();
	while (!glfwWindowShouldClose(m_window)) {
		double cur_time = glfwGetTime();

		glfwPollEvents();

		if (m_ui_state == UIStates::kOctreeTracer)
			m_camera->Control(m_window, float(cur_time - lst_time));

		ui_switch_state();

		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		if (m_ui_display_flag)
			ui_render_main();
		ImGui::Render();

		draw_frame();
		lst_time = cur_time;
	}
}

void Application::ui_switch_state() {
	if (m_path_tracer_thread->IsRunning()) {
		m_ui_state = UIStates::kPathTracing;
	} else if (m_loader_thread->IsRunning()) {
		m_ui_state = UIStates::kLoading;

		if (m_loader_thread->TryJoin()) {
			m_ui_state = m_octree->Empty() ? UIStates::kEmpty : UIStates::kOctreeTracer;
		}
	} else if (m_octree->Empty())
		m_ui_state = UIStates::kEmpty;
	else {
		m_ui_state = UIStates::kOctreeTracer;
	}
}

void Application::ui_render_main() {
	if (m_ui_state == UIStates::kLoading) {
		ImGui::OpenPopup(UI::kLoaderLoadingModal);
		UI::LoaderLoadingModal(m_loader_thread);
	}
	ui_menubar();

	UI::LoaderLoadSceneModal(m_loader_thread);
	UI::PathTracerStartModal(m_path_tracer_thread);
	UI::PathTracerStopModal(m_path_tracer_thread);
	UI::PathTracerExportEXRModal(m_path_tracer_thread);
	UI::LightingLoadEnvMapModal(m_main_command_pool, m_lighting);
}

void Application::ui_menubar() {
	const char *open_modal = nullptr;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::BeginMainMenuBar();

	if (m_ui_state == UIStates::kPathTracing)
		UI::PathTracerControlButtons(m_path_tracer_thread, &open_modal);
	else {
		UI::LoaderLoadButton(m_loader_thread, &open_modal);
		if (m_ui_state == UIStates::kOctreeTracer)
			UI::PathTracerStartButton(m_path_tracer_thread, &open_modal);
	}

	ImGui::Separator();

	if (m_ui_state == UIStates::kPathTracing)
		UI::PathTracerMenuItems(m_path_tracer_thread);
	else if (m_ui_state == UIStates::kOctreeTracer) {
		UI::OctreeTracerMenuItems(m_octree_tracer);
		UI::CameraMenuItems(m_camera);
		UI::LightingMenuItems(m_main_command_pool, m_lighting, &open_modal);
	}

	UI::LogMenuItems(m_log_sink);

	// Status bar
	if (m_ui_state == UIStates::kOctreeTracer)
		UI::OctreeTracerRightStatus(m_octree_tracer);
	else if (m_ui_state == UIStates::kPathTracing)
		UI::PathTracerRightStatus(m_path_tracer_thread);

	ImGui::EndMainMenuBar();
	ImGui::PopStyleVar();

	if (open_modal)
		ImGui::OpenPopup(open_modal);
}

void Application::glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	auto *app = (Application *)glfwGetWindowUserPointer(window);
	if (!ImGui::GetCurrentContext()->NavWindow ||
	    (ImGui::GetCurrentContext()->NavWindow->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus)) {
		if (action == GLFW_PRESS && key == GLFW_KEY_X)
			app->m_ui_display_flag ^= 1u;
	}
}

void Application::glfw_framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
	auto *app = (Application *)glfwGetWindowUserPointer(window);
	app->m_frame_manager->Resize();
}

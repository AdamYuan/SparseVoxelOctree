#include "Application.hpp"

#include "Config.hpp"
#include "ImGuiHelper.hpp"
#include <spdlog/spdlog.h>

#include <font-awesome/IconsFontAwesome5.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_internal.h>
#include <tinyexr.h>
#include <tinyfiledialogs.h>

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
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(kWidth, kHeight, kAppName, nullptr, nullptr);
	glfwSetWindowUserPointer(m_window, this);
	glfwSetKeyCallback(m_window, glfw_key_callback);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::LoadFontAwesome();
	ImGui::StyleCinder();
	ImGui_ImplGlfw_InitForVulkan(m_window, true);
}

void Application::create_render_pass() {
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = m_swapchain->GetImageFormat();
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

	std::vector<VkSubpassDependency> subpass_dependencies(2);
	subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpass_dependencies[0].dstSubpass = 0;
	subpass_dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependencies[0].srcAccessMask = 0;
	subpass_dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	subpass_dependencies[1].srcSubpass = 0;
	subpass_dependencies[1].dstSubpass = 1;
	subpass_dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	render_pass_info.dependencyCount = subpass_dependencies.size();
	render_pass_info.pDependencies = subpass_dependencies.data();

	m_render_pass = myvk::RenderPass::Create(m_device, render_pass_info);
}

void Application::create_framebuffers() {
	m_framebuffers.resize(m_swapchain->GetImageCount());
	for (uint32_t i = 0; i < m_swapchain->GetImageCount(); ++i) {
		m_framebuffers[i] = myvk::Framebuffer::Create(m_render_pass, m_swapchain_image_views[i]);
	}
}

void Application::draw_frame() {
	m_frame_manager.BeforeAcquire();
	uint32_t image_index;
	m_swapchain->AcquireNextImage(&image_index, m_frame_manager.GetAcquireDoneSemaphorePtr(), nullptr);
	m_frame_manager.AfterAcquire(image_index);

	uint32_t current_frame = m_frame_manager.GetCurrentFrame();
	m_camera->UpdateFrameUniformBuffer(current_frame);
	const std::shared_ptr<myvk::CommandBuffer> &command_buffer = m_frame_command_buffers[current_frame];

	command_buffer->Reset();
	command_buffer->Begin();

	if (m_ui_state == UIStates::kOctreeTracer) {
		m_octree_tracer->CmdBeamRenderPass(command_buffer, current_frame);
	}
	command_buffer->CmdBeginRenderPass(m_render_pass, m_framebuffers[image_index], {{{0.0f, 0.0f, 0.0f, 1.0f}}});
	if (m_ui_state == UIStates::kOctreeTracer) {
		m_octree_tracer->CmdDrawPipeline(command_buffer, current_frame);
	} else if (m_ui_state == UIStates::kPathTracing) {
		m_path_tracer_viewer->CmdDrawPipeline(command_buffer);
	}
	command_buffer->CmdNextSubpass();
	m_imgui_renderer.CmdDrawPipeline(command_buffer, current_frame);
	command_buffer->CmdEndRenderPass();
	command_buffer->End();

	m_frame_manager.BeforeSubmit();
	command_buffer->Submit(
	    {{m_frame_manager.GetAcquireDoneSemaphorePtr(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}},
	    {m_frame_manager.GetRenderDoneSemaphorePtr()}, m_frame_manager.GetFrameFencePtr());
	m_swapchain->Present(image_index, {m_frame_manager.GetRenderDoneSemaphorePtr()});
}

void Application::initialize_vulkan() {
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
		std::vector<myvk::QueueRequirement> queue_requirements = {
		    myvk::QueueRequirement(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT, &m_main_queue, m_surface,
		                           &m_present_queue),
		    myvk::QueueRequirement(VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT,
		                           &m_loader_queue),
		    myvk::QueueRequirement(VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, &m_path_tracer_queue),
		};
		myvk::DeviceCreateInfo device_create_info;
		device_create_info.Initialize(physical_devices[0], queue_requirements, {VK_KHR_SWAPCHAIN_EXTENSION_NAME});
		if (!device_create_info.QueueSupport()) {
			spdlog::error("Failed to find queues!");
			exit(EXIT_FAILURE);
		}
		if (!device_create_info.ExtensionSupport()) {
			spdlog::error("Failed to find extension support!");
			exit(EXIT_FAILURE);
		}
		m_device = myvk::Device::Create(device_create_info);
		if (!m_device) {
			spdlog::error("Failed to create logical device!");
			exit(EXIT_FAILURE);
		}
	}

	spdlog::info("Physical Device: {}", m_device->GetPhysicalDevicePtr()->GetProperties().deviceName);
	spdlog::info("Present Queue: ({}){}, Main Queue: ({}){}, Loader Queue: ({}){}, PathTracer Queue: ({}){}",
	             m_present_queue->GetFamilyIndex(), (void *)m_present_queue->GetHandle(),        // present queue
	             m_main_queue->GetFamilyIndex(), (void *)m_main_queue->GetHandle(),              // main queue
	             m_loader_queue->GetFamilyIndex(), (void *)m_loader_queue->GetHandle(),          // loader queue
	             m_path_tracer_queue->GetFamilyIndex(), (void *)m_path_tracer_queue->GetHandle() // path tracer queue
	);

	if (m_path_tracer_queue->GetFamilyIndex() == m_main_queue->GetFamilyIndex()) {
		spdlog::warn("Async path tracing queue not available, the main thread might be blocked when path tracing");
	}

	m_swapchain = myvk::Swapchain::Create(m_main_queue, m_present_queue, false);
	spdlog::info("Swapchain image count: {}", m_swapchain->GetImageCount());

	m_swapchain_images = myvk::SwapchainImage::Create(m_swapchain);
	m_swapchain_image_views.resize(m_swapchain->GetImageCount());
	for (uint32_t i = 0; i < m_swapchain->GetImageCount(); ++i)
		m_swapchain_image_views[i] = myvk::ImageView::Create(m_swapchain_images[i]);

	m_main_command_pool = myvk::CommandPool::Create(m_main_queue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	m_path_tracer_command_pool =
	    myvk::CommandPool::Create(m_path_tracer_queue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	m_frame_command_buffers = myvk::CommandBuffer::CreateMultiple(m_main_command_pool, kFrameCount);
}

Application::Application() {
	if (volkInitialize() != VK_SUCCESS) {
		spdlog::error("Failed to load vulkan!");
		exit(EXIT_FAILURE);
	}

	m_log_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(kLogLimit);
	m_log_sink->set_pattern("%H:%M:%S.%e"); // only display time
	spdlog::default_logger()->sinks().push_back(m_log_sink);

	create_window();
	initialize_vulkan();
	glfwSetWindowTitle(
	    m_window,
	    (std::string{kAppName} + " | " + m_device->GetPhysicalDevicePtr()->GetProperties().deviceName).c_str());
	create_render_pass();
	create_framebuffers();
	m_imgui_renderer.Initialize(m_main_command_pool, m_render_pass, 1, kFrameCount);

	m_camera = Camera::Create(m_device, kFrameCount);
	m_camera->m_position = glm::vec3(1.5);
	m_frame_manager.Initialize(m_swapchain, kFrameCount);
	m_octree = Octree::Create(m_device);
	m_octree_tracer = OctreeTracer::Create(m_octree, m_camera, m_render_pass, 0, kFrameCount);
	m_path_tracer = PathTracer::Create(m_octree, m_camera, m_path_tracer_command_pool);
	m_path_tracer_viewer = PathTracerViewer::Create(m_path_tracer, m_render_pass, 0);

	m_loader_thread = LoaderThread::Create(m_octree, m_loader_queue, m_main_queue);
	m_path_tracer_thread = PathTracerThread::Create(m_path_tracer_viewer, m_path_tracer_queue, m_main_queue);
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
	m_device->WaitIdle();
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
		ImGui::OpenPopup("Loading");
		ui_loading_modal();
	}
	ui_menubar();
}

void Application::ui_push_disable() {
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
}

void Application::ui_pop_disable() {
	ImGui::PopItemFlag();
	ImGui::PopStyleVar();
}

void Application::ui_menubar() {
	bool open_load_scene_popup = false, start_path_tracer_popup = false, stop_path_tracer_popup = false,
	     open_export_exr_popup = false;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::BeginMainMenuBar();

	if (m_ui_state == UIStates::kPathTracing) {
		if (ImGui::MenuItem(ICON_FA_FILE_EXPORT)) {
			open_export_exr_popup = true;
		}
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted("Export OpenEXR");
			ImGui::EndTooltip();
		}

		if (ImGui::MenuItem(ICON_FA_STOP)) {
			stop_path_tracer_popup = true;
		}
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted("Stop Path Tracer");
			ImGui::EndTooltip();
		}

		if (m_path_tracer_thread->IsPause()) {
			if (ImGui::MenuItem(ICON_FA_PLAY)) {
				m_path_tracer_thread->SetPause(false);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::TextUnformatted("Resume Path Tracer");
				ImGui::EndTooltip();
			}
		} else {
			if (ImGui::MenuItem(ICON_FA_PAUSE)) {
				m_path_tracer_thread->SetPause(true);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::TextUnformatted("Pause Path Tracer");
				ImGui::EndTooltip();
			}
		}
	} else {
		if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN))
			open_load_scene_popup = true;
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted("Load Scene");
			ImGui::EndTooltip();
		}
		if (m_ui_state == UIStates::kOctreeTracer) {
			if (ImGui::MenuItem(ICON_FA_PLAY)) {
				start_path_tracer_popup = true;
			}
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::TextUnformatted("Start Path Tracer");
				ImGui::EndTooltip();
			}
		}
	}

	ImGui::Separator();

	if (m_ui_state == UIStates::kPathTracing) {
		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Color", nullptr,
			                    m_path_tracer_viewer->m_view_type == PathTracerViewer::ViewTypes::kColor)) {
				m_path_tracer_viewer->m_view_type = PathTracerViewer::ViewTypes::kColor;
				m_path_tracer_thread->UpdateViewer();
			}
			if (ImGui::MenuItem("Albedo", nullptr,
			                    m_path_tracer_viewer->m_view_type == PathTracerViewer::ViewTypes::kAlbedo)) {
				m_path_tracer_viewer->m_view_type = PathTracerViewer::ViewTypes::kAlbedo;
				m_path_tracer_thread->UpdateViewer();
			}
			if (ImGui::MenuItem("Normal", nullptr,
			                    m_path_tracer_viewer->m_view_type == PathTracerViewer::ViewTypes::kNormal)) {
				m_path_tracer_viewer->m_view_type = PathTracerViewer::ViewTypes::kNormal;
				m_path_tracer_thread->UpdateViewer();
			}

			ImGui::EndMenu();
		}
	} else if (m_ui_state == UIStates::kOctreeTracer) {
		if (ImGui::BeginMenu("Camera")) {
			ImGui::DragAngle("FOV", &m_camera->m_fov, 1, 10, 179);
			ImGui::DragFloat("Speed", &m_camera->m_speed, 0.005f, 0.005f, 0.2f);
			ImGui::InputFloat3("Position", &m_camera->m_position[0]);
			ImGui::DragAngle("Yaw", &m_camera->m_yaw, 1, 0, 360);
			ImGui::DragAngle("Pitch", &m_camera->m_pitch, 1, -90, 90);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Diffuse", nullptr, m_octree_tracer->m_view_type == OctreeTracer::ViewTypes::kDiffuse))
				m_octree_tracer->m_view_type = OctreeTracer::ViewTypes::kDiffuse;
			if (ImGui::MenuItem("Normal", nullptr, m_octree_tracer->m_view_type == OctreeTracer::ViewTypes::kNormal))
				m_octree_tracer->m_view_type = OctreeTracer::ViewTypes::kNormal;
			if (ImGui::MenuItem("Iterations", nullptr,
			                    m_octree_tracer->m_view_type == OctreeTracer::ViewTypes::kIteration))
				m_octree_tracer->m_view_type = OctreeTracer::ViewTypes::kIteration;

			ImGui::Checkbox("Beam Optimization", &m_octree_tracer->m_beam_enable);
			ImGui::EndMenu();
		}
	}

	if (ImGui::BeginMenu("Log")) {
		const auto &logs_raw = m_log_sink->last_raw();
		const auto &logs_time = m_log_sink->last_formatted();

		static constexpr ImU32 kLogColors[7] = {0xffffffffu, 0xffffffffu, 0xff00bd00u, 0xff00ffffu,
		                                        0xff0000ffu, 0xff0000ffu, 0xffffffffu};

		static constexpr const char *kLogLevelStrs[7] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"};

		ImGuiTableFlags flags =
		    ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("Log Table", 4, flags, {kWidth / 2.0f, kHeight / 2.0f})) {
			ImGui::TableSetupColumn("Time");
			ImGui::TableSetupColumn("Level");
			ImGui::TableSetupColumn("Thread");
			ImGui::TableSetupColumn("Text");
			ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
			ImGui::TableHeadersRow();

			for (uint32_t i = 0; i < logs_raw.size(); ++i) {
				const auto &log = logs_raw[i];
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::TextUnformatted(logs_time[i].c_str());

				ImGui::TableSetColumnIndex(1);
				ImGui::PushStyleColor(ImGuiCol_Text, kLogColors[log.level]);
				ImGui::TextUnformatted(kLogLevelStrs[log.level]);
				ImGui::PopStyleColor();

				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%lu", log.thread_id);

				ImGui::TableSetColumnIndex(3);
				ImGui::TextUnformatted(log.payload.begin(), log.payload.end());
			}

			ImGui::EndTable();
		}
		ImGui::EndMenu();
	}

	// Status bar
	float indent_w = ImGui::GetWindowContentRegionWidth(), spacing = ImGui::GetStyle().ItemSpacing.x;

	char buf[128];
	if (m_ui_state == UIStates::kOctreeTracer) {
		sprintf(buf, "FPS %.1f", ImGui::GetIO().Framerate);
		indent_w -= ImGui::CalcTextSize(buf).x;
		ImGui::SameLine(indent_w);
		ImGui::TextUnformatted(buf);

		indent_w -= spacing;
		ImGui::SameLine(indent_w);
		ImGui::Separator();

		sprintf(buf, ICON_FA_CUBE " %d", m_octree->GetLevel());
		indent_w -= ImGui::CalcTextSize(buf).x + spacing;
		ImGui::SameLine(indent_w);
		ImGui::TextUnformatted(buf);

		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted("Octree Level");
			ImGui::EndTooltip();
		}

		sprintf(buf, ICON_FA_DATABASE " %.0f/%.0f MB", m_octree->GetRange() / 1000000.0f,
		        m_octree->GetBuffer()->GetSize() / 1000000.0f);
		indent_w -= ImGui::CalcTextSize(buf).x + spacing;
		ImGui::SameLine(indent_w);
		ImGui::TextUnformatted(buf);

		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted("Octree Used Size / Allocated Size");
			ImGui::EndTooltip();
		}
	} else if (m_ui_state == UIStates::kPathTracing) {
		sprintf(buf, "SPP %u", m_path_tracer_thread->GetSPP());
		indent_w -= ImGui::CalcTextSize(buf).x;
		ImGui::SameLine(indent_w);
		ImGui::TextUnformatted(buf);

		indent_w -= spacing;
		ImGui::SameLine(indent_w);
		ImGui::Separator();

		sprintf(buf, ICON_FA_BOLT " %u", m_path_tracer->m_bounce);
		indent_w -= ImGui::CalcTextSize(buf).x + spacing;
		ImGui::SameLine(indent_w);
		ImGui::TextUnformatted(buf);

		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted("Bounce");
			ImGui::EndTooltip();
		}

		sprintf(buf, ICON_FA_STOPWATCH " %u sec", uint32_t(m_path_tracer_thread->GetRenderTime()));
		indent_w -= ImGui::CalcTextSize(buf).x + spacing;
		ImGui::SameLine(indent_w);
		ImGui::TextUnformatted(buf);

		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted("Render Time");
			ImGui::EndTooltip();
		}

		if (m_path_tracer_queue->GetFamilyIndex() == m_main_queue->GetFamilyIndex()) {
			indent_w -= ImGui::CalcTextSize(ICON_FA_EXCLAMATION_TRIANGLE).x + spacing;
			ImGui::SameLine(indent_w);
			ImGui::TextUnformatted(ICON_FA_EXCLAMATION_TRIANGLE);

			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::TextUnformatted("Async path tracing queue not available");
				ImGui::EndTooltip();
			}
		}
	}

	ImGui::EndMainMenuBar();
	ImGui::PopStyleVar();

	if (open_load_scene_popup)
		ImGui::OpenPopup("Load Scene");
	if (open_export_exr_popup)
		ImGui::OpenPopup("Export OpenEXR");
	if (start_path_tracer_popup)
		ImGui::OpenPopup("Start PathTracer");
	if (stop_path_tracer_popup)
		ImGui::OpenPopup("Stop PathTracer");

	ui_load_scene_modal();
	ui_export_exr_modal();
	ui_start_path_tracer_modal();
	ui_stop_path_tracer_modal();
}

bool Application::ui_file_open(const char *label, const char *btn, char *buf, size_t buf_size, const char *title,
                               int filter_num, const char *const *filter_patterns) {
	bool ret = ImGui::InputText(label, buf, buf_size);
	ImGui::SameLine();

	if (ImGui::Button(btn)) {
		const char *filename = tinyfd_openFileDialog(title, "", filter_num, filter_patterns, nullptr, false);
		if (filename)
			strcpy(buf, filename);
		ret = true;
	}
	return ret;
}

bool Application::ui_file_save(const char *label, const char *btn, char *buf, size_t buf_size, const char *title,
                               int filter_num, const char *const *filter_patterns) {
	bool ret = ImGui::InputText(label, buf, buf_size);
	ImGui::SameLine();

	if (ImGui::Button(btn)) {
		const char *filename = tinyfd_saveFileDialog(title, "", filter_num, filter_patterns, nullptr);
		if (filename)
			strcpy(buf, filename);
		ret = true;
	}
	return ret;
}

void Application::ui_load_scene_modal() {
	if (ImGui::BeginPopupModal("Load Scene", nullptr,
	                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoMove)) {
		static char name_buf[kFilenameBufSize];
		static int octree_leve = 10;

		constexpr const char *kFilter[] = {"*.obj"};

		ui_file_open("OBJ Filename", "...##5", name_buf, kFilenameBufSize, "OBJ Filename", 1, kFilter);
		ImGui::DragInt("Octree Level", &octree_leve, 1, 2, 12);

		if (ImGui::Button("Load", ImVec2(256, 0))) {
			m_loader_thread->Launch(name_buf, octree_leve);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(256, 0)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

void Application::ui_loading_modal() {
	if (ImGui::BeginPopupModal("Loading", nullptr,
	                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoMove)) {
		ImGui::Spinner("##spinner", 12, 6, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
		ImGui::SameLine();
		ImGui::Text("Loading ...");

		ImGui::EndPopup();
	}
}

void Application::ui_start_path_tracer_modal() {
	if (ImGui::BeginPopupModal("Start PathTracer", nullptr,
	                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoMove)) {
		int bounce = m_path_tracer->m_bounce;
		if (ImGui::DragInt("Bounce", &bounce, 1, kMinBounce, kMaxBounce))
			m_path_tracer->m_bounce = bounce;

		ImGui::DragFloat3("Sun Radiance", &m_path_tracer->m_sun_radiance[0], 0.1f, 0.0f, kMaxSunRadiance);

		if (ImGui::Button("Start", ImVec2(256, 0))) {
			m_path_tracer_thread->Launch();

			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(256, 0)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

void Application::ui_stop_path_tracer_modal() {
	if (ImGui::BeginPopupModal("Stop PathTracer", nullptr,
	                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoMove)) {
		ImGui::Text("Are you sure?");
		if (ImGui::Button("Stop", ImVec2(64, 0))) {
			m_path_tracer_thread->StopAndJoin();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(64, 0)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

void Application::ui_export_exr_modal() {
	if (ImGui::BeginPopupModal("Export OpenEXR", nullptr,
	                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoMove)) {
		constexpr const char *kChannels[] = {"Color", "Albedo", "Normal"};
		static const char *const *current_channel = kChannels + 0;
		if (ImGui::BeginCombo("Channel", *current_channel)) {
			for (int n = 0; n < IM_ARRAYSIZE(kChannels); n++) {
				bool is_selected = (current_channel == kChannels + n);
				if (ImGui::Selectable(kChannels[n], is_selected))
					current_channel = kChannels + n;
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		static char exr_name_buf[kFilenameBufSize]{};
		static bool save_as_fp16{false};

		constexpr const char *kFilter[] = {"*.exr"};
		ui_file_save("OpenEXR Filename", "...##0", exr_name_buf, kFilenameBufSize, "Export OpenEXR", 1, kFilter);

		ImGui::Checkbox("Export as FP16", &save_as_fp16);

		{
			if (ImGui::Button("Export", ImVec2(256, 0))) {
				bool tmp_pause = m_path_tracer_thread->IsPause();
				m_path_tracer_thread->SetPause(true);

				std::vector<float> pixels;
				{
					std::lock_guard<std::mutex> lock_guard{m_path_tracer_thread->GetTargetMutex()};
					if (current_channel == kChannels + 0) // color
						pixels = m_path_tracer->ExtractColorImage(m_path_tracer_command_pool);
					else if (current_channel == kChannels + 1) // albedo
						pixels = m_path_tracer->ExtractAlbedoImage(m_path_tracer_command_pool);
					else // normal
						pixels = m_path_tracer->ExtractNormalImage(m_path_tracer_command_pool);
				}

				m_path_tracer_thread->SetPause(tmp_pause);

				char *err{nullptr};
				if (SaveEXR(pixels.data(), kWidth, kHeight, 3, save_as_fp16, exr_name_buf, (const char **)&err) < 0)
					spdlog::error("{}", err);
				else
					spdlog::info("Saved EXR image to {}", exr_name_buf);
				free(err);

				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(256, 0))) {
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
}

void Application::glfw_key_callback(GLFWwindow *window, int key, int, int action, int) {
	auto *app = (Application *)glfwGetWindowUserPointer(window);
	if (!ImGui::GetCurrentContext()->NavWindow ||
	    (ImGui::GetCurrentContext()->NavWindow->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus)) {
		if (action == GLFW_PRESS && key == GLFW_KEY_X)
			app->m_ui_display_flag ^= 1u;
	}
}

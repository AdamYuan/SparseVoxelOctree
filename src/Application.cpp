#include "Application.hpp"

#include <plog/Log.h>
#include "Config.hpp"
#include "ShaderSrc.hpp"
#include "Voxelizer.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_glfw.h>
#include <tinyfiledialogs.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_type,
	const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
	void *user_data) {
	if (message_severity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		LOGE.printf("%s", callback_data->pMessage);
	else if (message_severity >=
			 VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		LOGW.printf("%s", callback_data->pMessage);
	else LOGI.printf("%s", callback_data->pMessage);
	return VK_FALSE;
}

void Application::create_window() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(kWidth, kHeight, kAppName, nullptr, nullptr);
	glfwSetWindowUserPointer(m_window, this);
	glfwSetKeyCallback(m_window, glfw_key_callback);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDarcula();
	ImGui_ImplGlfw_InitForVulkan(m_window, true);
}

void Application::create_depth_buffer() {
	m_depth_image = myvk::Image::CreateTexture2D(m_device, m_swapchain->GetExtent(), 1, VK_FORMAT_D32_SFLOAT,
												 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	m_depth_image_view = myvk::ImageView::Create(m_depth_image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);

	std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(m_device);
	std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(m_graphics_compute_command_pool);
	command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VkImageSubresourceRange range = {};
	range.layerCount = 1;
	range.baseArrayLayer = 0;
	range.levelCount = 1;
	range.baseMipLevel = 0;
	range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	command_buffer->CmdPipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
									   {}, {}, m_depth_image->GetMemoryBarriers({range}, 0,
																				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
																				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
																				VK_IMAGE_LAYOUT_UNDEFINED,
																				VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL));
	command_buffer->End();
	command_buffer->Submit({}, {}, fence);
	fence->Wait();
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

	VkAttachmentDescription depth_attachment = {};
	depth_attachment.format = m_depth_image->GetFormat();
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::vector<VkSubpassDescription> subpasses(2);
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &color_attachment_ref;
	subpasses[0].pDepthStencilAttachment = &depth_attachment_ref;

	subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[1].colorAttachmentCount = 1;
	subpasses[1].pColorAttachments = &color_attachment_ref;
	subpasses[1].pDepthStencilAttachment = nullptr;

	std::array<VkAttachmentDescription, 2> attachments = {color_attachment, depth_attachment};

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = attachments.size();
	render_pass_info.pAttachments = attachments.data();
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

void Application::create_descriptor_set_layout() {
	VkDescriptorSetLayoutBinding ubo_layout_binding = {};
	ubo_layout_binding.binding = 0;
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(m_device, {ubo_layout_binding});
}

void Application::create_graphics_pipeline() {
	m_pipeline_layout = myvk::PipelineLayout::Create(m_device,
													 {m_descriptor_set_layout, m_scene.GetDescriptorSetLayoutPtr()},
													 {Scene::GetDefaultPushConstantRange()});
	std::shared_ptr<myvk::ShaderModule> vert_shader_module, frag_shader_module;
	vert_shader_module = myvk::ShaderModule::Create(m_device, (uint32_t *) kTriVertSpv, sizeof(kTriVertSpv));
	frag_shader_module = myvk::ShaderModule::Create(m_device, (uint32_t *) kTriFragSpv, sizeof(kTriFragSpv));

	VkPipelineShaderStageCreateInfo vert_shader_stage_info = vert_shader_module->GetPipelineShaderStageCreateInfo(
		VK_SHADER_STAGE_VERTEX_BIT);
	VkPipelineShaderStageCreateInfo frag_shader_stage_info = frag_shader_module->GetPipelineShaderStageCreateInfo(
		VK_SHADER_STAGE_FRAGMENT_BIT);

	VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

	auto binding_descriptor = Scene::GetVertexBindingDescription();
	auto attribute_descriptions = Scene::GetVertexAttributeDescriptions();
	VkPipelineVertexInputStateCreateInfo vertex_input = {};
	vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input.vertexBindingDescriptionCount = 1;
	vertex_input.pVertexBindingDescriptions = &binding_descriptor;
	vertex_input.vertexAttributeDescriptionCount = attribute_descriptions.size();
	vertex_input.pVertexAttributeDescriptions = attribute_descriptions.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) m_swapchain->GetExtent().width;
	viewport.height = (float) m_swapchain->GetExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = m_swapchain->GetExtent();

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = VK_TRUE;
	depth_stencil.depthWriteEnable = VK_TRUE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.stencilTestEnable = VK_FALSE;

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDynamicState = nullptr;
	pipeline_info.subpass = 0;

	m_pipeline = myvk::GraphicsPipeline::Create(m_pipeline_layout, m_render_pass, pipeline_info);
}

void Application::create_framebuffers() {
	m_framebuffers.resize(m_swapchain->GetImageCount());

	for (uint32_t i = 0; i < m_swapchain->GetImageCount(); ++i) {
		m_framebuffers[i] = myvk::Framebuffer::Create(m_render_pass, {m_swapchain_image_views[i], m_depth_image_view},
													  m_swapchain->GetExtent());
	}
}

void Application::create_descriptor_pool() {
	constexpr uint32_t kPoolSizeCount = 1;
	VkDescriptorPoolSize pool_sizes[kPoolSizeCount] = {
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, kFrameCount}
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = kFrameCount;
	pool_info.poolSizeCount = kPoolSizeCount;
	pool_info.pPoolSizes = pool_sizes;

	m_descriptor_pool = myvk::DescriptorPool::Create(m_device, pool_info);
}

void Application::create_frame_resources() {
	m_frame_resources.resize(kFrameCount);

	std::vector<std::shared_ptr<myvk::CommandBuffer>> command_buffers = myvk::CommandBuffer::CreateMultiple(
		m_graphics_compute_command_pool, kFrameCount);
	std::vector<std::shared_ptr<myvk::DescriptorSet>> descriptor_sets = myvk::DescriptorSet::CreateMultiple(
		m_descriptor_pool,
		std::vector<std::shared_ptr<myvk::DescriptorSetLayout>>(kFrameCount, m_descriptor_set_layout));

	for (uint32_t i = 0; i < kFrameCount; ++i) {
		m_frame_resources[i].m_uniform_buffer = myvk::Buffer::Create(m_device, sizeof(SVkCamera),
																	 VMA_MEMORY_USAGE_CPU_TO_GPU,
																	 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_frame_resources[i].m_descriptor_set = descriptor_sets[i];
		m_frame_resources[i].m_descriptor_set->UpdateUniformBuffer(m_frame_resources[i].m_uniform_buffer, 0);
		m_frame_resources[i].m_command_buffer = command_buffers[i];
	}
}

void Application::draw_frame() {
	m_frame_manager.BeforeAcquire();
	uint32_t image_index;
	m_swapchain->AcquireNextImage(&image_index, m_frame_manager.GetAcquireDoneSemaphorePtr(), nullptr);
	m_frame_manager.AfterAcquire(image_index);

	FrameResource &current_resource = m_frame_resources[m_frame_manager.GetCurrentFrame()];
	current_resource.m_uniform_buffer->UpdateData(m_camera.GetBuffer());

	const std::shared_ptr<myvk::CommandBuffer> &command_buffer = current_resource.m_command_buffer;

	command_buffer->Reset();
	command_buffer->Begin();
	std::vector<VkClearValue> clear_values(2);
	clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
	clear_values[1].depthStencil = {1.0f, 0};
	command_buffer->CmdBeginRenderPass(m_render_pass, m_framebuffers[image_index], clear_values,
									   m_swapchain->GetExtent());
	{
		if (m_pipeline) {
			command_buffer->CmdBindPipeline(m_pipeline);
			command_buffer->CmdBindDescriptorSets({current_resource.m_descriptor_set, m_scene.GetDescriptorSetPtr()},
												  m_pipeline, {});
			m_scene.CmdDraw(command_buffer, m_pipeline_layout);
		}

		command_buffer->CmdNextSubpass();

		m_imgui_renderer.CmdDrawSubpass(command_buffer, m_frame_manager.GetCurrentFrame());

	}
	command_buffer->CmdEndRenderPass();
	command_buffer->End();

	m_frame_manager.BeforeSubmit();
	current_resource.m_command_buffer->Submit(
		{{m_frame_manager.GetAcquireDoneSemaphorePtr(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}},
		{m_frame_manager.GetRenderDoneSemaphorePtr()},
		m_frame_manager.GetFrameFencePtr());
	m_swapchain->Present(image_index, {m_frame_manager.GetRenderDoneSemaphorePtr()});
}

void Application::initialize_vulkan() {
	m_instance = myvk::Instance::CreateWithGlfwExtensions(true, debug_callback);
	if (!m_instance) {
		LOGE.printf("Failed to create instance!");
		exit(EXIT_FAILURE);
	}

	std::vector<std::shared_ptr<myvk::PhysicalDevice>> physical_devices = myvk::PhysicalDevice::Fetch(m_instance);
	if (physical_devices.empty()) {
		LOGE.printf("Failed to find physical device with vulkan support!");
		exit(EXIT_FAILURE);
	}

	m_surface = myvk::Surface::Create(m_instance, m_window);
	if (!m_surface) {
		LOGE.printf("Failed to create surface!");
		exit(EXIT_FAILURE);
	}

	//DEVICE CREATION
	{
		std::vector<myvk::QueueRequirement> queue_requirements = {
			myvk::QueueRequirement(VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT, &m_graphics_compute_queue,
								   m_surface, &m_present_queue),
			myvk::QueueRequirement(VK_QUEUE_COMPUTE_BIT, &m_async_compute_queue)
		};
		myvk::DeviceCreateInfo device_create_info;
		device_create_info.Initialize(physical_devices[0], queue_requirements, {VK_KHR_SWAPCHAIN_EXTENSION_NAME});
		if (!device_create_info.QueueSupport()) {
			LOGE.printf("Failed to find queues!");
			exit(EXIT_FAILURE);
		}
		if (!device_create_info.ExtensionSupport()) {
			LOGE.printf("Failed to find extension support!");
			exit(EXIT_FAILURE);
		}
		m_device = myvk::Device::Create(device_create_info);
		if (!m_device) {
			LOGE.printf("Failed to create logical device!");
			exit(EXIT_FAILURE);
		}
	}
	LOGI.printf("Physical Device: %s", m_device->GetPhysicalDevicePtr()->GetProperties().deviceName);
	LOGI.printf("Present Queue: %p, Graphics|Compute Queue: %p, Async Compute Queue: %p",
				m_present_queue->GetHandle(),
				m_graphics_compute_queue->GetHandle(),
				m_async_compute_queue->GetHandle());

	if (m_async_compute_queue->GetHandle() == m_graphics_compute_queue->GetHandle()) {
		LOGE.printf("No separate Compute Queue support");
	} else if (m_async_compute_queue->GetFamilyIndex() == m_graphics_compute_queue->GetFamilyIndex()) {
		LOGW.printf("Async Compute Queue is not fully asynchronous");
	}

	m_graphics_compute_command_pool = myvk::CommandPool::Create(m_graphics_compute_queue,
																VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	m_swapchain = myvk::Swapchain::Create(m_graphics_compute_queue, m_present_queue, false);

	LOGI.printf("Swapchain image count: %u", m_swapchain->GetImageCount());

	m_swapchain_images = myvk::SwapchainImage::Create(m_swapchain);
	m_swapchain_image_views.resize(m_swapchain->GetImageCount());
	for (uint32_t i = 0; i < m_swapchain->GetImageCount(); ++i)
		m_swapchain_image_views[i] = myvk::ImageView::Create(m_swapchain_images[i]);
}


Application::Application() {
	if (volkInitialize() != VK_SUCCESS) {
		LOGE.printf("Failed to load vulkan!");
		exit(EXIT_FAILURE);
	}

	create_window();
	initialize_vulkan();

	m_frame_manager.Initialize(m_swapchain, kFrameCount);
	create_depth_buffer();
	create_render_pass();
	m_imgui_renderer.Initialize(m_graphics_compute_command_pool, m_render_pass, 1, kFrameCount);
	create_descriptor_set_layout();
	create_framebuffers();
	create_descriptor_pool();
	create_frame_resources();

	LoadScene("/home/adamyuan/Projects/Adypt/models/sponza/sponza.obj", 10);
	//LoadScene("/home/adamyuan/Projects/Adypt/models/San_Miguel/san-miguel-mod.obj", 10);
}

void Application::LoadScene(const char *filename, uint32_t octree_level) {
	m_device->WaitIdle();
	if (m_scene.Initialize(m_graphics_compute_queue, filename)) {
		Voxelizer voxelizer;
		voxelizer.Initialize(m_device, m_scene, octree_level);
		voxelizer.CountAndCreateFragmentList(m_graphics_compute_command_pool);
		std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(m_device);
		std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(m_graphics_compute_command_pool);
		command_buffer->Begin();
		voxelizer.CmdVoxelize(command_buffer);
		command_buffer->End();
		command_buffer->Submit({}, {}, fence);
		fence->Wait();
		LOGV.printf("Voxel fragment list filled");
		create_graphics_pipeline();
	}
}

void Application::Run() {
	double lst_time = glfwGetTime();
	while (!glfwWindowShouldClose(m_window)) {
		double cur_time = glfwGetTime();

		glfwPollEvents();

		m_camera.Control(m_window, float(cur_time - lst_time));
		m_camera.UpdateMatrices();

		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ui_main();
		ImGui::Render();

		draw_frame();
		lst_time = cur_time;
	}
	m_device->WaitIdle();
}

void Application::ui_main() {
	ui_main_menubar();
	ui_info_overlay();
}

void Application::ui_push_disable() {
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
}

void Application::ui_pop_disable() {
	ImGui::PopItemFlag();
	ImGui::PopStyleVar();
}

void Application::ui_info_overlay() {
	ImGui::SetNextWindowPos(ImVec2(10.0f, ImGui::GetIO().DisplaySize.y - 10.0f),
							ImGuiCond_Always, ImVec2(0, 1));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.4f)); // Transparent background
	if (ImGui::Begin("INFO", nullptr,
					 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
					 | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove
					 | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
		ImGui::Text("Toggle UI display with [X]");
		//ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
		//ImGui::Text("OpenGL version: %s", glGetString(GL_VERSION));
		ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);

		/*if(m_octree)
			ImGui::Text("Octree Level: %d", m_octree->GetLevel());

		if(m_pathtracing_flag)
			ImGui::Text("SPP: %d", m_pathtracer.GetSPP());*/

		ImGui::End();
	}
	ImGui::PopStyleColor();
}

void Application::ui_main_menubar() {
	bool open_load_scene_popup = false, open_export_exr_popup = false;

	ImGui::BeginMainMenuBar();

	if (!m_pathtracing_flag) {
		if (ImGui::Button("Load Scene"))
			open_load_scene_popup = true;

		/*if(m_octree && ImGui::Button("Start PT"))
		{
			m_pathtracing_flag = true;
			m_pathtracer.Prepare(m_camera, *m_octree, m_octree_tracer);
		}*/

		if (ImGui::BeginMenu("Camera")) {
			ImGui::DragAngle("FOV", &m_camera.m_fov, 1, 10, 180);
			ImGui::DragFloat("Speed", &m_camera.m_speed, 0.005f, 0.005f, 0.2f);
			ImGui::InputFloat3("Position", &m_camera.m_position[0]);
			ImGui::DragAngle("Yaw", &m_camera.m_yaw, 1, 0, 360);
			ImGui::DragAngle("Pitch", &m_camera.m_pitch, 1, -90, 90);
			ImGui::EndMenu();
		}

		/*if(ImGui::BeginMenu("Primary View"))
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
			ImGui::DragFloat3("Sun Radiance", &m_pathtracer.m_sun_radiance[0], 0.1f, 0.0f, 20.0f);
			ImGui::EndMenu();
		}*/
	}
	/*else if(m_octree)
	{
		if(ImGui::Button("Exit PT"))
			m_pathtracing_flag = false;
		if(ImGui::Button("Export OpenEXR"))
			open_export_exr_popup = true;

		ImGui::Checkbox("Pause", &m_pathtracer.m_pause);

		if(ImGui::BeginMenu("View"))
		{
			if(ImGui::MenuItem("Color", nullptr, m_pathtracer.m_view_type == PathTracer::kColor))
				m_pathtracer.m_view_type = PathTracer::kColor;
			if(ImGui::MenuItem("Albedo", nullptr, m_pathtracer.m_view_type == PathTracer::kAlbedo))
				m_pathtracer.m_view_type = PathTracer::kAlbedo;
			if(ImGui::MenuItem("Normal", nullptr, m_pathtracer.m_view_type == PathTracer::kNormal))
				m_pathtracer.m_view_type = PathTracer::kNormal;

			ImGui::EndMenu();
		}
	}*/

	ImGui::EndMainMenuBar();

	if (open_load_scene_popup)
		ImGui::OpenPopup("Load Scene");
	if (open_export_exr_popup)
		ImGui::OpenPopup("Export OpenEXR");

	ui_load_scene_modal();
	ui_export_exr_modal();
}

bool
Application::ui_file_open(const char *label, const char *btn, char *buf, size_t buf_size, const char *title,
						  int filter_num, const char *const *filter_patterns) {
	bool ret = ImGui::InputText(label, buf, buf_size);
	ImGui::SameLine();

	if (ImGui::Button(btn)) {
		const char *filename = tinyfd_openFileDialog(title, "", filter_num, filter_patterns,
													 nullptr, false);
		if (filename) strcpy(buf, filename);
		ret = true;
	}
	return ret;
}

bool
Application::ui_file_save(const char *label, const char *btn, char *buf, size_t buf_size, const char *title,
						  int filter_num, const char *const *filter_patterns) {
	bool ret = ImGui::InputText(label, buf, buf_size);
	ImGui::SameLine();

	if (ImGui::Button(btn)) {
		const char *filename = tinyfd_saveFileDialog(title, "", filter_num, filter_patterns,
													 nullptr);
		if (filename) strcpy(buf, filename);
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

		ui_file_open("OBJ Filename", "...##5", name_buf, kFilenameBufSize, "OBJ Filename",
					 1, kFilter);
		ImGui::DragInt("Octree Level", &octree_leve, 1, 2, 12);

		if (ImGui::Button("Load", ImVec2(256, 0))) {
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

void Application::ui_export_exr_modal() {
	/*if (ImGui::BeginPopupModal("Export OpenEXR", nullptr,
							   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
	{
		static char exr_name_buf[kFilenameBufSize]{};
		static bool save_as_fp16{false};
		ImGui::LabelText("", "INFO: will export %s channel",
						 m_pathtracer.m_view_type == PathTracer::kColor ? "COLOR" :
						 (m_pathtracer.m_view_type == PathTracer::kAlbedo ? "ALBEDO" : "NORMAL"));

		constexpr const char *kFilter[] = {"*.exr"};
		ui_file_save("OpenEXR Filename", "...##0", exr_name_buf, kFilenameBufSize, "Export OpenEXR",
					 1, kFilter);

		ImGui::Checkbox("Export As FP16", &save_as_fp16);

		{
			if (ImGui::Button("Export", ImVec2(256, 0)))
			{
				m_pathtracer.Save(exr_name_buf, save_as_fp16);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(256, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}*/
}

void Application::glfw_key_callback(GLFWwindow *window, int key, int, int action, int) {
	auto *app = (Application *) glfwGetWindowUserPointer(window);
	if (!ImGui::GetCurrentContext()->NavWindow
		|| (ImGui::GetCurrentContext()->NavWindow->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus)) {
		if (action == GLFW_PRESS && key == GLFW_KEY_X)
			app->m_ui_display_flag ^= 1u;
	}
}




#include "Config.hpp"
#include "PathTracerThread.hpp"
#include <spdlog/spdlog.h>

std::shared_ptr<PathTracerThread> PathTracerThread::Create(const std::shared_ptr<PathTracerViewer> &path_tracer_viewer,
                                                           const std::shared_ptr<myvk::Queue> &m_path_tracer_queue,
                                                           const std::shared_ptr<myvk::Queue> &main_queue) {
	std::shared_ptr<PathTracerThread> ret = std::make_shared<PathTracerThread>();
	ret->m_path_tracer_viewer_ptr = path_tracer_viewer;
	ret->m_path_tracer_queue = m_path_tracer_queue;
	ret->m_main_queue = main_queue;
	return ret;
}

PathTracerThread::~PathTracerThread() { StopAndJoin(); }

void PathTracerThread::Launch() {
	if (IsRunning())
		return;
	m_run = true;
	m_pause = false;
	m_spp = 0;
	m_time = glfwGetTime();

	m_path_tracer_viewer_ptr->GetPathTracerPtr()->Reset(myvk::CommandPool::Create(m_path_tracer_queue));
	m_path_tracer_viewer_ptr->Reset(myvk::CommandPool::Create(m_main_queue));

	m_path_tracer_thread = std::thread(&PathTracerThread::path_tracer_thread_func, this);
	m_viewer_thread = std::thread(&PathTracerThread::viewer_thread_func, this);
}

double PathTracerThread::GetRenderTime() const {
	if (m_pause)
		return m_time;
	return glfwGetTime() - m_time;
}

void PathTracerThread::SetPause(bool pause) {
	if (pause == m_pause)
		return;
	m_time = glfwGetTime() - m_time;
	m_pause.store(pause, std::memory_order_release);

	m_pause_condition_variable.notify_one();
}

void PathTracerThread::StopAndJoin() {
	if (!IsRunning())
		return;
	m_run.store(false, std::memory_order_release);
	UpdateViewer();
	SetPause(false);
	m_path_tracer_thread.join();
	m_viewer_thread.join();
}

void PathTracerThread::UpdateViewer() {
	m_viewer_condition_variable.notify_one();
}

void PathTracerThread::path_tracer_thread_func() {
	spdlog::info("Enter path tracer thread");

	std::shared_ptr<myvk::Device> device = m_main_queue->GetDevicePtr();
	std::shared_ptr<PathTracer> path_tracer = m_path_tracer_viewer_ptr->GetPathTracerPtr();

	std::shared_ptr<myvk::CommandPool> pt_command_pool = myvk::CommandPool::Create(m_path_tracer_queue);

	std::shared_ptr<myvk::CommandBuffer> pt_command_buffer = myvk::CommandBuffer::Create(pt_command_pool);
	pt_command_buffer->Begin();
	path_tracer->CmdRender(pt_command_buffer);
	pt_command_buffer->End();

	std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(device);
	while (m_run.load(std::memory_order_acquire)) {
		while (m_pause.load(std::memory_order_acquire)) {
			std::unique_lock<std::mutex> lock{m_pause_mutex};
			m_pause_condition_variable.wait(lock);
		}

		if (!m_run.load(std::memory_order_acquire))
			break;

		{
			fence->Reset();
			pt_command_buffer->Submit(fence);
			fence->Wait();
		}

		if ((m_spp++) % kPTResultUpdateInterval == 0) {
			UpdateViewer();
		}
	}

	spdlog::info("Quit path tracer thread");
}

void PathTracerThread::viewer_thread_func() {
	spdlog::info("Enter path tracer viewer thread");

	std::shared_ptr<myvk::Device> device = m_main_queue->GetDevicePtr();
	std::shared_ptr<PathTracer> path_tracer = m_path_tracer_viewer_ptr->GetPathTracerPtr();

	std::shared_ptr<myvk::CommandPool> pt_command_pool = myvk::CommandPool::Create(m_path_tracer_queue);
	std::shared_ptr<myvk::CommandBuffer> pt_release_command_buffer = myvk::CommandBuffer::Create(pt_command_pool);
	std::shared_ptr<myvk::CommandBuffer> pt_acquire_command_buffer = myvk::CommandBuffer::Create(pt_command_pool);

	if (m_path_tracer_queue->GetFamilyIndex() != m_main_queue->GetFamilyIndex()) {
		pt_release_command_buffer->Begin();
		pt_release_command_buffer->CmdPipelineBarrier(
		    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {}, {},
		    {path_tracer->GetColorImage()->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_IMAGE_LAYOUT_GENERAL,
		                                                    VK_IMAGE_LAYOUT_GENERAL, m_path_tracer_queue, m_main_queue),
		     path_tracer->GetAlbedoImage()->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_IMAGE_LAYOUT_GENERAL,
		                                                     VK_IMAGE_LAYOUT_GENERAL, m_path_tracer_queue,
		                                                     m_main_queue),
		     path_tracer->GetNormalImage()->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_IMAGE_LAYOUT_GENERAL,
		                                                     VK_IMAGE_LAYOUT_GENERAL, m_path_tracer_queue,
		                                                     m_main_queue)});
		pt_release_command_buffer->End();

		pt_acquire_command_buffer->Begin();
		pt_acquire_command_buffer->CmdPipelineBarrier(
		    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {}, {},
		    {path_tracer->GetColorImage()->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_IMAGE_LAYOUT_GENERAL,
		                                                    VK_IMAGE_LAYOUT_GENERAL, m_main_queue, m_path_tracer_queue),
		     path_tracer->GetAlbedoImage()->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_IMAGE_LAYOUT_GENERAL,
		                                                     VK_IMAGE_LAYOUT_GENERAL, m_main_queue,
		                                                     m_path_tracer_queue),
		     path_tracer->GetNormalImage()->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_IMAGE_LAYOUT_GENERAL,
		                                                     VK_IMAGE_LAYOUT_GENERAL, m_main_queue,
		                                                     m_path_tracer_queue)});
		pt_acquire_command_buffer->End();
	}

	// TODO: Test queue ownership transfer
	std::shared_ptr<myvk::CommandPool> main_command_pool = myvk::CommandPool::Create(m_main_queue);

	std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(device);
	while (m_run.load(std::memory_order_acquire)) {
		{
			std::unique_lock<std::mutex> lock{m_viewer_mutex};
			m_viewer_condition_variable.wait(lock);
		}

		if (!m_run.load(std::memory_order_acquire))
			break;

		// release pt queue ownership
		if (m_path_tracer_queue->GetFamilyIndex() != m_main_queue->GetFamilyIndex()) {
			fence->Reset();
			pt_release_command_buffer->Submit(fence);
			fence->Wait();
		}

		{ // gen render pass
			std::shared_ptr<myvk::CommandBuffer> viewer_command_buffer = myvk::CommandBuffer::Create(main_command_pool);
			viewer_command_buffer->Begin();
			viewer_command_buffer->CmdPipelineBarrier(
			    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, {}, {},
			    {path_tracer->GetColorImage()->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_SHADER_READ_BIT,
			                                                    VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
			                                                    m_path_tracer_queue, m_main_queue),
			     path_tracer->GetAlbedoImage()->GetMemoryBarrier(
			         VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
			         VK_IMAGE_LAYOUT_GENERAL, m_path_tracer_queue, m_main_queue),
			     path_tracer->GetNormalImage()->GetMemoryBarrier(
			         VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
			         VK_IMAGE_LAYOUT_GENERAL, m_path_tracer_queue, m_main_queue)});
			m_path_tracer_viewer_ptr->CmdGenRenderPass(viewer_command_buffer);
			viewer_command_buffer->CmdPipelineBarrier(
			    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {}, {},
			    {path_tracer->GetColorImage()->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0,
			                                                    VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
			                                                    m_main_queue, m_path_tracer_queue),
			     path_tracer->GetAlbedoImage()->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0,
			                                                     VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
			                                                     m_main_queue, m_path_tracer_queue),
			     path_tracer->GetNormalImage()->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0,
			                                                     VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
			                                                     m_main_queue, m_path_tracer_queue)});
			viewer_command_buffer->End();

			fence->Reset();
			viewer_command_buffer->Submit(fence);
			fence->Wait();
		}

		// acquire pt queue ownership
		if (m_path_tracer_queue->GetFamilyIndex() != m_main_queue->GetFamilyIndex()) {
			fence->Reset();
			pt_acquire_command_buffer->Submit({}, {}, fence);
			fence->Wait();
		}
	}

	spdlog::info("Quit path tracer viewer thread");
}

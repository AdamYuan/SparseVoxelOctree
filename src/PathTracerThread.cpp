#include "PathTracerThread.hpp"
#include "Config.hpp"
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

	m_path_tracer_viewer_ptr->GetPathTracerPtr()->Reset(myvk::CommandPool::Create(m_path_tracer_queue), m_main_queue);
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

	spdlog::debug("m_pause_semaphore signal");
	m_pause_semaphore.signal();
}

void PathTracerThread::StopAndJoin() {
	if (!IsRunning())
		return;

	m_pause.store(false, std::memory_order_release);
	m_run.store(false, std::memory_order_release);

	spdlog::debug("m_viewer_semaphore signal");
	m_viewer_semaphore.signal();
	spdlog::debug("m_pause_semaphore signal");
	m_pause_semaphore.signal();

	m_path_tracer_thread.join();
	m_viewer_thread.join();
}

void PathTracerThread::UpdateViewer() {
	spdlog::debug("m_viewer_semaphore signal");
	m_viewer_semaphore.signal();
}

void PathTracerThread::path_tracer_thread_func() {
	spdlog::info("Enter path tracer thread");

	m_main_queue->WaitIdle();

	const std::shared_ptr<myvk::Device> &device = m_main_queue->GetDevicePtr();
	const std::shared_ptr<PathTracer> &path_tracer = m_path_tracer_viewer_ptr->GetPathTracerPtr();

	std::shared_ptr<myvk::CommandPool> pt_command_pool = myvk::CommandPool::Create(m_path_tracer_queue);

	std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(device);

	std::shared_ptr<myvk::CommandBuffer> pt_command_buffer = myvk::CommandBuffer::Create(pt_command_pool);
	pt_command_buffer->Begin();
	path_tracer->CmdRender(pt_command_buffer);
	pt_command_buffer->End();

	acquire_resources();

	while (m_run.load(std::memory_order_acquire)) {
		fence->Reset();
		pt_command_buffer->Submit(fence);
		fence->Wait();

		if ((m_spp++) % kPTResultUpdateInterval == 0)
			UpdateViewer();

		while (m_pause.load(std::memory_order_acquire)) {
			spdlog::debug("m_pause_semaphore wait");
			m_pause_semaphore.wait();
		}
	}

	release_resources();

	spdlog::info("Quit path tracer thread");
}

void PathTracerThread::viewer_thread_func() {
	spdlog::info("Enter path tracer viewer thread");

	const std::shared_ptr<myvk::Device> &device = m_main_queue->GetDevicePtr();
	const std::shared_ptr<PathTracer> &path_tracer = m_path_tracer_viewer_ptr->GetPathTracerPtr();

	std::shared_ptr<myvk::CommandPool> main_command_pool = myvk::CommandPool::Create(m_main_queue);

	std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(device);
	while (m_run.load(std::memory_order_acquire)) {
		{
			std::shared_ptr<myvk::CommandBuffer> viewer_command_buffer = myvk::CommandBuffer::Create(main_command_pool);
			viewer_command_buffer->Begin();
			m_path_tracer_viewer_ptr->CmdGenRenderPass(viewer_command_buffer);
			viewer_command_buffer->End();

			fence->Reset();
			viewer_command_buffer->Submit(fence);
			fence->Wait();
		}

		spdlog::debug("m_viewer_semaphore wait");
		m_viewer_semaphore.wait();
	}

	spdlog::info("Quit path tracer viewer thread");
}

void PathTracerThread::acquire_resources() {
	if (m_main_queue->GetFamilyIndex() != m_path_tracer_queue->GetFamilyIndex()) {
		spdlog::info("Acquire resources to path tracer queue");

		const std::shared_ptr<myvk::Device> &device = m_main_queue->GetDevicePtr();
		const std::shared_ptr<PathTracer> &path_tracer = m_path_tracer_viewer_ptr->GetPathTracerPtr();

		std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(device);

		std::shared_ptr<myvk::CommandPool> main_command_pool = myvk::CommandPool::Create(m_main_queue);
		std::shared_ptr<myvk::CommandPool> pt_command_pool = myvk::CommandPool::Create(m_path_tracer_queue);

		std::shared_ptr<myvk::CommandBuffer> release_command_buffer = myvk::CommandBuffer::Create(main_command_pool);
		std::shared_ptr<myvk::CommandBuffer> acquire_command_buffer = myvk::CommandBuffer::Create(pt_command_pool);

		release_command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		path_tracer->GetOctreePtr()->CmdTransferOwnership(release_command_buffer, m_main_queue->GetFamilyIndex(),
		                                                  m_path_tracer_queue->GetFamilyIndex());
		path_tracer->GetLightingPtr()->GetEnvironmentMapPtr()->CmdTransferOwnership(
		    release_command_buffer, m_main_queue->GetFamilyIndex(), m_path_tracer_queue->GetFamilyIndex());
		release_command_buffer->End();

		acquire_command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		path_tracer->GetOctreePtr()->CmdTransferOwnership(acquire_command_buffer, m_main_queue->GetFamilyIndex(),
		                                                  m_path_tracer_queue->GetFamilyIndex());
		path_tracer->GetLightingPtr()->GetEnvironmentMapPtr()->CmdTransferOwnership(
		    acquire_command_buffer, m_main_queue->GetFamilyIndex(), m_path_tracer_queue->GetFamilyIndex());
		acquire_command_buffer->End();

		fence->Reset();
		std::shared_ptr<myvk::Semaphore> semaphore = myvk::Semaphore::Create(device);
		release_command_buffer->Submit({}, {semaphore});
		acquire_command_buffer->Submit({{semaphore, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT}}, {}, fence);
		fence->Wait();
	}
}
void PathTracerThread::release_resources() {
	if (m_main_queue->GetFamilyIndex() != m_path_tracer_queue->GetFamilyIndex()) {
		spdlog::info("Release resources to main queue");

		const std::shared_ptr<myvk::Device> &device = m_main_queue->GetDevicePtr();
		const std::shared_ptr<PathTracer> &path_tracer = m_path_tracer_viewer_ptr->GetPathTracerPtr();

		std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(device);

		std::shared_ptr<myvk::CommandPool> main_command_pool = myvk::CommandPool::Create(m_main_queue);
		std::shared_ptr<myvk::CommandPool> pt_command_pool = myvk::CommandPool::Create(m_path_tracer_queue);

		std::shared_ptr<myvk::CommandBuffer> release_command_buffer = myvk::CommandBuffer::Create(pt_command_pool);
		std::shared_ptr<myvk::CommandBuffer> acquire_command_buffer = myvk::CommandBuffer::Create(main_command_pool);

		release_command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		path_tracer->GetOctreePtr()->CmdTransferOwnership(release_command_buffer, m_path_tracer_queue->GetFamilyIndex(),
		                                                  m_main_queue->GetFamilyIndex());
		path_tracer->GetLightingPtr()->GetEnvironmentMapPtr()->CmdTransferOwnership(
		    release_command_buffer, m_path_tracer_queue->GetFamilyIndex(), m_main_queue->GetFamilyIndex());
		release_command_buffer->End();

		acquire_command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		path_tracer->GetOctreePtr()->CmdTransferOwnership(acquire_command_buffer, m_path_tracer_queue->GetFamilyIndex(),
		                                                  m_main_queue->GetFamilyIndex());
		path_tracer->GetLightingPtr()->GetEnvironmentMapPtr()->CmdTransferOwnership(
		    acquire_command_buffer, m_path_tracer_queue->GetFamilyIndex(), m_main_queue->GetFamilyIndex());
		acquire_command_buffer->End();

		fence->Reset();
		std::shared_ptr<myvk::Semaphore> semaphore = myvk::Semaphore::Create(device);
		release_command_buffer->Submit({}, {semaphore});
		acquire_command_buffer->Submit({{semaphore, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT}}, {}, fence);
		fence->Wait();
	}
}

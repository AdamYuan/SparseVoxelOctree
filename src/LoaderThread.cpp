#include "LoaderThread.hpp"
#include <spdlog/spdlog.h>

std::shared_ptr<LoaderThread> LoaderThread::Create(const std::shared_ptr<Octree> &octree,
                                                   const std::shared_ptr<myvk::Queue> &loader_queue,
                                                   const std::shared_ptr<myvk::Queue> &main_queue) {
	std::shared_ptr<LoaderThread> ret = std::make_shared<LoaderThread>();
	ret->m_octree_ptr = octree;
	ret->m_loader_queue = loader_queue;
	ret->m_main_queue = main_queue;

	return ret;
}

void LoaderThread::Launch(const char *filename, uint32_t octree_level) {
	if (IsRunning())
		return;
	m_promise = std::promise<std::shared_ptr<OctreeBuilder>>();
	m_future = m_promise.get_future();
	m_thread = std::thread(&LoaderThread::thread_func, this, filename, octree_level);
}

bool LoaderThread::TryJoin() {
	if (!IsRunning())
		return false;

	if (m_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
		return false;

	m_thread.join();

	std::shared_ptr<OctreeBuilder> builder = m_future.get();
	if (builder) {
		std::shared_ptr<myvk::CommandPool> loader_command_pool = myvk::CommandPool::Create(m_loader_queue);
		m_main_queue->WaitIdle();
		m_octree_ptr->Update(loader_command_pool, builder);
		spdlog::info("Octree range: {} ({} MB)", m_octree_ptr->GetRange(), m_octree_ptr->GetRange() / 1000000.0f);
	}

	return true;
}

void LoaderThread::thread_func(const char *filename, uint32_t octree_level) {
	spdlog::info("Enter loader thread");
	m_notification = "";

	std::shared_ptr<myvk::Device> device = m_main_queue->GetDevicePtr();
	std::shared_ptr<myvk::CommandPool> main_command_pool = myvk::CommandPool::Create(m_main_queue);
	std::shared_ptr<myvk::CommandPool> loader_command_pool = myvk::CommandPool::Create(m_loader_queue);

	std::shared_ptr<Scene> scene;
	if ((scene = Scene::Create(m_loader_queue, filename, &m_notification))) {
		std::shared_ptr<Voxelizer> voxelizer = Voxelizer::Create(scene, loader_command_pool, octree_level);
		std::shared_ptr<OctreeBuilder> builder = OctreeBuilder::Create(voxelizer, loader_command_pool);

		std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(device);
		std::shared_ptr<myvk::QueryPool> query_pool = myvk::QueryPool::Create(device, VK_QUERY_TYPE_TIMESTAMP, 4);
		std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(loader_command_pool);
		command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		command_buffer->CmdResetQueryPool(query_pool);

		command_buffer->CmdWriteTimestamp(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 0);
		voxelizer->CmdVoxelize(command_buffer);
		command_buffer->CmdWriteTimestamp(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, query_pool, 1);

		command_buffer->CmdPipelineBarrier(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		                                   {},
		                                   {voxelizer->GetVoxelFragmentList()->GetMemoryBarrier(
		                                       VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT)},
		                                   {});

		command_buffer->CmdWriteTimestamp(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 2);
		builder->CmdBuild(command_buffer);
		command_buffer->CmdWriteTimestamp(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, query_pool, 3);

		if (m_main_queue->GetFamilyIndex() != m_loader_queue->GetFamilyIndex()) {
			builder->CmdTransferOctreeOwnership(command_buffer, m_loader_queue->GetFamilyIndex(),
			                                    m_main_queue->GetFamilyIndex(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			                                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		}

		command_buffer->End();

		m_notification = "Voxelizing and Building Octree";
		spdlog::info("Voxelize and Octree building BEGIN");

		command_buffer->Submit(fence);
		fence->Wait();

		// time measurement
		uint64_t timestamps[4];
		query_pool->GetResults64(timestamps, VK_QUERY_RESULT_WAIT_BIT);
		spdlog::info("Voxelize and Octree building FINISHED in {} ms (Voxelize "
		             "{} ms, Octree building {} ms)",
		             double(timestamps[3] - timestamps[0]) * 0.000001, double(timestamps[1] - timestamps[0]) * 0.000001,
		             double(timestamps[3] - timestamps[2]) * 0.000001);

		if (m_main_queue->GetFamilyIndex() != m_loader_queue->GetFamilyIndex()) {
			command_buffer = myvk::CommandBuffer::Create(main_command_pool);
			command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			builder->CmdTransferOctreeOwnership(command_buffer, m_loader_queue->GetFamilyIndex(),
			                                    m_main_queue->GetFamilyIndex());
			command_buffer->End();

			fence->Reset();
			command_buffer->Submit(fence);
			fence->Wait();
		}

		m_promise.set_value(builder);
	} else
		m_promise.set_value(nullptr);

	spdlog::info("Quit loader thread");
}

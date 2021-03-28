#ifndef LOADER_THREAD_HPP
#define LOADER_THREAD_HPP

#include "Octree.hpp"
#include "OctreeBuilder.hpp"
#include "myvk/Queue.hpp"
#include <atomic>
#include <future>
#include <memory>
#include <thread>

class LoaderThread {
private:
	std::shared_ptr<Octree> m_octree_ptr;
	std::shared_ptr<myvk::Queue> m_loader_queue, m_main_queue;

	std::thread m_thread;
	std::promise<std::shared_ptr<OctreeBuilder>> m_promise;
	std::future<std::shared_ptr<OctreeBuilder>> m_future;

	std::atomic<const char *> m_notification;

	void thread_func(const char *filename, uint32_t octree_level);

public:
	static std::shared_ptr<LoaderThread> Create(const std::shared_ptr<Octree> &octree,
	                                            const std::shared_ptr<myvk::Queue> &loader_queue,
	                                            const std::shared_ptr<myvk::Queue> &main_queue);
	const std::shared_ptr<Octree> &GetOctreePtr() const { return m_octree_ptr; }
	const std::shared_ptr<myvk::Queue> &GetLoaderQueue() const { return m_loader_queue; }
	const std::shared_ptr<myvk::Queue> &GetMainQueue() const { return m_main_queue; }

	const char *GetNotification() const { return m_notification; }

	void Launch(const char *filename, uint32_t octree_level);
	bool TryJoin();

	bool IsRunning() const { return m_thread.joinable(); }
};

#endif

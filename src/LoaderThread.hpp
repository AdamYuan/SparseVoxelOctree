#ifndef LOADER_THREAD_HPP
#define LOADER_THREAD_HPP

#include "Octree.hpp"
#include "myvk/Queue.hpp"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

class LoaderThread {
private:
	std::shared_ptr<Octree> m_octree_ptr;
	std::shared_ptr<myvk::Queue> m_loader_queue, m_main_queue;

	std::thread m_thread;
	std::mutex m_mutex;
	bool m_job_done;
	std::condition_variable m_condition_variable;

	void thread_func(const char *filename, uint32_t octree_level);

public:
	static std::shared_ptr<LoaderThread> Create(const std::shared_ptr<Octree> &octree,
	                                            const std::shared_ptr<myvk::Queue> &loader_queue,
	                                            const std::shared_ptr<myvk::Queue> &main_queue);
	const std::shared_ptr<Octree> &GetOctreePtr() const { return m_octree_ptr; }

	void Launch(const char *filename, uint32_t octree_level);
	bool TryJoin();

	bool IsRunning() const { return m_thread.joinable(); }
};

#endif

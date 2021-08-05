#ifndef PATH_TRACER_THREAD_HPP
#define PATH_TRACER_THREAD_HPP

#include "PathTracerViewer.hpp"
#include "myvk/Queue.hpp"
#include <atomic>
#include <binary_semaphore.hpp>
#include <memory>
#include <thread>

class PathTracerThread {
private:
	std::shared_ptr<PathTracerViewer> m_path_tracer_viewer_ptr;
	std::shared_ptr<myvk::Queue> m_path_tracer_queue, m_main_queue;

	std::thread m_path_tracer_thread, m_viewer_thread;
	binary_semaphore m_pause_semaphore, m_viewer_semaphore;
	std::atomic_bool m_pause, m_run;

	uint32_t m_spp;
	double m_time;

	void path_tracer_thread_func();
	void viewer_thread_func();

	void acquire_resources();
	void release_resources();

public:
	static std::shared_ptr<PathTracerThread> Create(const std::shared_ptr<PathTracerViewer> &path_tracer_viewer,
	                                                const std::shared_ptr<myvk::Queue> &path_tracer_queue,
	                                                const std::shared_ptr<myvk::Queue> &main_queue);
	const std::shared_ptr<PathTracerViewer> &GetPathTracerViewerPtr() const { return m_path_tracer_viewer_ptr; }
	const std::shared_ptr<myvk::Queue> &GetPathTracerQueue() const { return m_path_tracer_queue; }
	const std::shared_ptr<myvk::Queue> &GetMainQueue() const { return m_main_queue; }
	~PathTracerThread();

	void Launch();
	void SetPause(bool pause);
	void StopAndJoin();

	void UpdateViewer();

	uint32_t GetSPP() const { return m_spp; }
	double GetRenderTime() const;

	bool IsPause() const { return m_pause; }
	bool IsRunning() const { return m_path_tracer_thread.joinable() || m_viewer_thread.joinable(); }
};

#endif

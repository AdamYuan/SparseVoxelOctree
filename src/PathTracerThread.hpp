#ifndef PATH_TRACER_THREAD_HPP
#define PATH_TRACER_THREAD_HPP

#include "PathTracerViewer.hpp"
#include "myvk/Queue.hpp"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

class PathTracerThread {
private:
	std::shared_ptr<PathTracerViewer> m_path_tracer_viewer_ptr;
	std::shared_ptr<myvk::Queue> m_path_tracer_queue, m_main_queue;

	std::thread m_thread;
	std::condition_variable m_pause_condition_variable;
	std::mutex m_pause_mutex, m_target_mutex;
	bool m_pause, m_run;

	uint32_t m_spp;
	double m_start_time;

	void thread_func();

public:
	static std::shared_ptr<PathTracerThread> Create(const std::shared_ptr<PathTracerViewer> &path_tracer_viewer,
	                                                const std::shared_ptr<myvk::Queue> &path_tracer_queue,
	                                                const std::shared_ptr<myvk::Queue> &main_queue);
	const std::shared_ptr<PathTracerViewer> &GetPathTracerViewerPtr() const { return m_path_tracer_viewer_ptr; }
	~PathTracerThread();

	void Launch();
	void SetPause(bool pause);
	void StopAndJoin();

	uint32_t GetSPP() const { return m_spp; }
	double GetRenderTime() const;

	std::mutex &GetTargetMutex() { return m_target_mutex; }
	const std::mutex &GetTargetMutex() const { return m_target_mutex; }

	bool IsPause() const { return m_pause; }
	bool IsRunning() const { return m_thread.joinable(); }
};

#endif

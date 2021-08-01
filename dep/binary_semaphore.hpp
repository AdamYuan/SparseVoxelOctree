#ifndef BINARY_SEMAPHORE_HPP
#define BINARY_SEMAPHORE_HPP

#include <condition_variable>
#include <mutex>

class binary_semaphore {
public:
	inline binary_semaphore() : m_state(false) {}
	inline explicit binary_semaphore(bool state) : m_state(state) {}

	inline void wait() {
		std::unique_lock<std::mutex> lk(m_mtx);
		m_cv.wait(lk, [=] { return m_state; });
		m_state = false;
	}
	inline bool try_wait() {
		std::lock_guard<std::mutex> lk(m_mtx);
		if (m_state) {
			m_state = false;
			return true;
		} else {
			return false;
		}
	}
	inline void signal() {
		std::lock_guard<std::mutex> lk(m_mtx);
		if (!m_state) {
			m_state = true;
			m_cv.notify_one();
		}
	}

private:
	bool m_state;
	std::mutex m_mtx;
	std::condition_variable m_cv;
};

#endif

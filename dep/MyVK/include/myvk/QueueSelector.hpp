#ifndef MYVK_QUEUE_SELECTOR_HPP
#define MYVK_QUEUE_SELECTOR_HPP

#include "myvk/Ptr.hpp"
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <volk.h>

namespace myvk {

class Device;
class Queue;
class PresentQueue;
class Surface;
class PhysicalDevice;
class QueueSelection {
private:
	Ptr<Queue> *m_p_queue;
	uint32_t m_family, m_index_specifier;
	Ptr<Surface> m_surface;

public:
	inline QueueSelection(Ptr<Queue> *p_queue, uint32_t family, uint32_t index_specifier)
	    : m_p_queue{p_queue}, m_family{family}, m_index_specifier{index_specifier} {}

	inline uint32_t GetFamily() const { return m_family; }
	inline uint32_t GetIndexSpecifier() const { return m_index_specifier; }
	inline Ptr<Queue> &GetQueueTargetRef() { return *m_p_queue; }
	inline const Ptr<Surface> &GetSurfacePtr() const { return m_surface; }
	inline bool IsPresentQueue() const { return m_surface != nullptr; }
	inline Ptr<PresentQueue> &GetPresentQueueTargetRef() { return *((Ptr<PresentQueue> *)m_p_queue); }

#ifdef MYVK_ENABLE_GLFW
public:
	inline QueueSelection(const Ptr<Surface> &surface, Ptr<PresentQueue> *p_present_queue, uint32_t family,
	                      uint32_t index_specifier)
	    : m_p_queue{(Ptr<Queue> *)p_present_queue}, m_surface{surface}, m_family{family}, m_index_specifier{
	                                                                                          index_specifier} {}
#endif
};

using QueueSelectorFunc = std::function<std::vector<QueueSelection>(const Ptr<const PhysicalDevice> &)>;

// default queue selectors
class GenericQueueSelector {
private:
	Ptr<Queue> *m_p_generic_queue;

public:
	inline explicit GenericQueueSelector(Ptr<Queue> *p_generic_queue) : m_p_generic_queue{p_generic_queue} {}
	std::vector<QueueSelection> operator()(const Ptr<const PhysicalDevice> &) const;
};

#ifdef MYVK_ENABLE_GLFW
// default queue selectors
class GenericPresentQueueSelector {
private:
	Ptr<Surface> m_surface;
	Ptr<Queue> *m_p_generic_queue;
	Ptr<PresentQueue> *m_p_present_queue;

public:
	GenericPresentQueueSelector(Ptr<Queue> *p_generic_queue, const Ptr<Surface> &surface,
	                            Ptr<PresentQueue> *p_present_queue);
	std::vector<QueueSelection> operator()(const Ptr<const PhysicalDevice> &) const;
};

// default queue selectors
class GenericPresentTransferQueueSelector {
private:
	Ptr<Surface> m_surface;
	Ptr<Queue> *m_p_generic_queue, *m_p_transfer_queue;
	Ptr<PresentQueue> *m_p_present_queue;

public:
	GenericPresentTransferQueueSelector(Ptr<Queue> *p_generic_queue, Ptr<Queue> *p_transfer_queue,
	                                    const Ptr<Surface> &surface, Ptr<PresentQueue> *p_present_queue);
	std::vector<QueueSelection> operator()(const Ptr<const PhysicalDevice> &) const;
};
#endif

} // namespace myvk

#endif

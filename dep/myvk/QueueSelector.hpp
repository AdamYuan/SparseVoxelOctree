#ifndef MYVK_QUEUE_SELECTOR_HPP
#define MYVK_QUEUE_SELECTOR_HPP

#include "PhysicalDevice.hpp"
#include <functional>
#include <memory>
#include <vector>

namespace myvk {

class Queue;
class PresentQueue;
class Surface;
struct QueueSelection {
	std::shared_ptr<Queue> *target;
	uint32_t family, index_specifier;
};
struct PresentQueueSelection {
	std::shared_ptr<PresentQueue> *target;
	std::shared_ptr<Surface> surface;
	uint32_t family, index_specifier;
};
using QueueSelectorFunc =
    std::function<bool(const std::shared_ptr<PhysicalDevice> &, std::vector<QueueSelection> *const,
                       std::vector<PresentQueueSelection> *const)>;

// default queue selectors
class GraphicsPresentQueueSelector {
private:
	std::shared_ptr<Surface> m_surface;
	std::shared_ptr<Queue> *m_graphics_queue;
	std::shared_ptr<PresentQueue> *m_present_queue;

public:
	GraphicsPresentQueueSelector(std::shared_ptr<Queue> *graphics_queue, const std::shared_ptr<Surface> &surface,
	                             std::shared_ptr<PresentQueue> *present_queue);
	bool operator()(const std::shared_ptr<PhysicalDevice> &, std::vector<QueueSelection> *const,
	                std::vector<PresentQueueSelection> *const) const;
};

} // namespace myvk

#endif

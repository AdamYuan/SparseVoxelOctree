#ifndef RESOURCE_TRACKER_HPP
#define RESOURCE_TRACKER_HPP

#include "Fence.hpp"
#include <map>

namespace myvk {
class ObjectTracker {
private:
	std::multimap<std::shared_ptr<Fence>, std::vector<std::shared_ptr<DeviceObjectBase>>> m_tracker;

public:
	void Track(const std::shared_ptr<Fence> &fence, const std::vector<std::shared_ptr<DeviceObjectBase>> &objects);

	void Update();

	void Join();

	~ObjectTracker();
};
} // namespace myvk

#endif

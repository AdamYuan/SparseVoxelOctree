#ifndef RESOURCE_TRACKER_HPP
#define RESOURCE_TRACKER_HPP

#include "Fence.hpp"
#include <map>

namespace myvk {
class ObjectTracker {
private:
	std::multimap<Ptr<Fence>, std::vector<Ptr<DeviceObjectBase>>> m_tracker;

public:
	void Track(const Ptr<Fence> &fence, const std::vector<Ptr<DeviceObjectBase>> &objects);

	void Update();

	void Join();

	~ObjectTracker();
};
} // namespace myvk

#endif

#include "myvk/ObjectTracker.hpp"

namespace myvk {
void ObjectTracker::Track(const Ptr<Fence> &fence,
                          const std::vector<Ptr<DeviceObjectBase>> &objects) {
	m_tracker.insert({fence, objects});
}

void ObjectTracker::Update() {
	for (auto i = m_tracker.begin(); i != m_tracker.end();) {
		if (i->first->Signaled())
			i = m_tracker.erase(i);
		else
			++i;
	}
}

void ObjectTracker::Join() {
	if (m_tracker.empty())
		return;
	std::vector<Ptr<Fence>> fences;
	for (const auto &i : m_tracker)
		fences.push_back(i.first);
	FenceGroup(fences).Wait();
	m_tracker.clear();
}

ObjectTracker::~ObjectTracker() { Join(); }
} // namespace myvk

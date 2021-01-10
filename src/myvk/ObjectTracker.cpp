#include "ObjectTracker.hpp"

namespace myvk {
void ObjectTracker::Track(const std::shared_ptr<Fence> &fence,
                          const std::vector<std::shared_ptr<DeviceObjectBase>> &objects) {
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
	std::vector<std::shared_ptr<Fence>> fences;
	for (const auto &i : m_tracker)
		fences.push_back(i.first);
	FenceGroup(fences).Wait();
	m_tracker.clear();
}

ObjectTracker::~ObjectTracker() { Join(); }
} // namespace myvk

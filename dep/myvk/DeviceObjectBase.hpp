#ifndef MYVK_DEVICE_OBJECT_BASE_HPP
#define MYVK_DEVICE_OBJECT_BASE_HPP

#include "Device.hpp"

namespace myvk {
class DeviceObjectBase {
public:
	virtual const std::shared_ptr<Device> &GetDevicePtr() const = 0;
};
} // namespace myvk

#endif

#ifndef MYVK_DEVICE_OBJECT_BASE_HPP
#define MYVK_DEVICE_OBJECT_BASE_HPP

#include "Device.hpp"
#include "Ptr.hpp"

namespace myvk {
class DeviceObjectBase : public Base {
public:
	virtual const Ptr<Device> &GetDevicePtr() const = 0;
	~DeviceObjectBase() override = default;
};
} // namespace myvk

#endif

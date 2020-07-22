#ifndef MYVK_QUEUE_HPP
#define MYVK_QUEUE_HPP

#include "DeviceObjectBase.hpp"

#include <volk.h>
#include <cinttypes>
#include <memory>

namespace myvk {
	class Queue : public DeviceObjectBase { //only can be created with device creation
	protected:
		std::shared_ptr<Device> m_device_ptr;

		VkQueue m_queue{nullptr};
		uint32_t m_family_index;

		static std::shared_ptr<Queue>
		create(const std::shared_ptr<Device> &device, uint32_t family_index, uint32_t queue_index);

	public:
		const std::shared_ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

		VkQueue GetHandle() const { return m_queue; }

		uint32_t GetFamilyIndex() const { return m_family_index; }

		VkResult WaitIdle() const;

		friend class DeviceCreateInfo;
	};

	class Surface;

	class PresentQueue : public Queue { //only can be created with device creation
	private:
		std::shared_ptr<Surface> m_surface_ptr;

		static std::shared_ptr<PresentQueue>
		create(const std::shared_ptr<Device> &device, const std::shared_ptr<Surface> &surface, uint32_t family_index,
			   uint32_t queue_index);

	public:
		const std::shared_ptr<Surface> &GetSurfacePtr() const { return m_surface_ptr; }

		friend class DeviceCreateInfo;
	};
}

#endif

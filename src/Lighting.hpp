#ifndef LIGHTING_HPP
#define LIGHTING_HPP

#include "EnvironmentMap.hpp"
#include <glm/glm.hpp>

class Lighting {
public:
	enum class LightTypes { kConstantColor = 0, kEnvironmentMap } m_light_type = LightTypes::kConstantColor;
	glm::vec3 m_sun_radiance;

private:
	std::shared_ptr<EnvironmentMap> m_environment_map_ptr;

public:
	static std::shared_ptr<Lighting> Create(const std::shared_ptr<EnvironmentMap> &environment_map);
	const std::shared_ptr<EnvironmentMap> &GetEnvironmentMapPtr() const { return m_environment_map_ptr; }
	LightTypes GetFinalLightType() const {
		return (m_light_type == LightTypes::kEnvironmentMap && m_environment_map_ptr->Empty())
		           ? LightTypes::kConstantColor
		           : m_light_type;
	}
};

#endif

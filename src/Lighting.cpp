#include "Lighting.hpp"
#include "Config.hpp"

std::shared_ptr<Lighting> Lighting::Create(const std::shared_ptr<EnvironmentMap> &environment_map) {
	std::shared_ptr<Lighting> ret = std::make_shared<Lighting>();
	ret->m_environment_map_ptr = environment_map;
	ret->m_sun_radiance = glm::vec3(kDefaultConstantColor);

	return ret;
}

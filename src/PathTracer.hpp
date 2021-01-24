#ifndef PATH_TRACER_HPP
#define PATH_TRACER_HPP

#include "Sobol.hpp"
#include "myvk/Image.hpp"

class PathTracer {
private:
	Sobol m_sobol;
	std::shared_ptr<myvk::Image> m_color_image, m_albedo_image, m_normal_image;
	std::shared_ptr<myvk::ImageView> m_color_image_view, m_albedo_image_view, m_normal_image_view;

public:
	void Initialize(const std::shared_ptr<myvk::Device> &device);
};

#endif

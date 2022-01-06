#ifndef DIFFUSE_GLSL
#define DIFFUSE_GLSL

#include "util.glsl"

#define DIFFUSE_BSDF (1.0 / PI)
vec3 Diffuse_Sample(in const vec3 normal, in const vec2 samp, out float pdf) {
	// cosine hemisphere sampling
	float r = sqrt(samp.x), phi = 2 * PI * samp.y;
	vec3 d = vec3(r * cos(phi), r * sin(phi), sqrt(1.0 - samp.x));
	// calculate pdf (dot(n, d) / PI)
	pdf = d.z / PI;
	return AlignDirection(d, normal);
}
float Diffuse_PDF(in const float ndd) { return ndd / PI; }


#endif

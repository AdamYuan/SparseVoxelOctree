#ifndef ENVIRONMENT_MAP_HPP
#define ENVIRONMENT_MAP_HPP

#ifndef ENVIRONMENT_MAP_SET
#define ENVIRONMENT_MAP_SET 2
#endif

#ifndef ENVIRONMENT_MAP_ENABLE_SAMPLE
#define ENVIRONMENT_MAP_ENABLE_SAMPLE 0
#endif

#include "util.glsl"

layout(set = ENVIRONMENT_MAP_SET, binding = 0) uniform sampler2D uEnvironmentMap;

vec2 EnvMap_Dir2Coord(in const vec3 d, in const float rot) {
	return vec2((atan(d.x, d.z) + rot) / PI * 0.5, acos(d.y) / PI);
}
vec3 EnvMap_Radiance(in const vec3 d, in const float rot) {
	return texture(uEnvironmentMap, EnvMap_Dir2Coord(d, rot)).xyz;
}
float EnvMap_PDF(in const vec3 d, in const float rot) {
	return texture(uEnvironmentMap, EnvMap_Dir2Coord(d, rot)).w / (2 * PI * PI * sqrt(1 - d.y * d.y));
}

#if ENVIRONMENT_MAP_ENABLE_SAMPLE == 1
layout(set = ENVIRONMENT_MAP_SET, binding = 1) uniform usampler2D uEnvMapAliasTable;
ivec2 env_map_dim = textureSize(uEnvironmentMap, 0).xy; // global variable for environmap dimension
ivec2 EnvMap_IndexToPixel(in const uint idx) { return ivec2(idx % env_map_dim.x, idx / env_map_dim.x); }
// EnvMapSample: returns direction
vec3 EnvMap_Sample(in const vec2 samp, in const float rot, out vec3 light, out float pdf) {
	uint sz = env_map_dim.x * env_map_dim.y;

	// sample alias table
	uint idx = clamp(uint(samp.x * float(sz)), 0u, sz - 1u), prob = uint(samp.y * 4294967296.0);
	ivec2 pix = EnvMap_IndexToPixel(idx);
	uvec2 alias = texelFetch(uEnvMapAliasTable, pix, 0).xy;
	if (prob > alias.x)
		pix = EnvMap_IndexToPixel(alias.y);

	// calculate dir and pdf
	vec2 uv = (vec2(pix) + 0.5) / vec2(env_map_dim);
	float phi = 2.0 * PI * uv.x - rot, theta = PI * uv.y;
	float sin_theta = sin(theta);
	vec4 texel = texelFetch(uEnvironmentMap, pix, 0);
	light = texel.xyz;
	pdf = texel.w / (2 * PI * PI * sin_theta);
	return vec3(sin_theta * sin(phi), cos(theta), sin_theta * cos(phi));
}
#endif

#endif

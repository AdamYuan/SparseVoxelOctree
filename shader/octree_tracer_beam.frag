#version 450 core
#define OCTREE_SET 0
#include "octree.glsl"
#define CAMERA_SET 1
#include "camera.glsl"
layout(push_constant) uniform uuPushConstant {
	uint uWidth, uHeight, uBeamSize;
	float uOriginSize;
};
layout(location = 0) out float oFragT;

vec3 GenCoarseRay() { return Camera_GenRay(ivec2(gl_FragCoord.xy) / vec2(uWidth, uHeight) * uBeamSize); }
vec3 GenCoarseRay(in vec2 bias) {
	return Camera_GenRay((vec2(ivec2(gl_FragCoord.xy)) + bias) / vec2(uWidth, uHeight) * uBeamSize);
}

float RayTangent(in vec3 x, in vec3 y) {
	float c = dot(x, y);
	return sqrt(1.0 - c * c) / abs(c);
}

void main() {
	vec3 o = uPosition.xyz, d = GenCoarseRay();
	float t0 = RayTangent(d, GenCoarseRay(vec2(0.5, 0.5)));
	float t1 = RayTangent(d, GenCoarseRay(vec2(-0.5, 0.5)));
	float t2 = RayTangent(d, GenCoarseRay(vec2(0.5, -0.5)));
	float t3 = RayTangent(d, GenCoarseRay(vec2(-0.5, -0.5)));
	float dir_sz = 2.0 * max(max(t0, t1), max(t2, t3)), t, size;
	oFragT = Octree_RayMarchCoarse(o, d, uOriginSize, dir_sz, t, size) ? max(0.0, t - size) : 1e10;
}

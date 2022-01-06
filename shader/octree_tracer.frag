#version 450
#define OCTREE_SET 0
#include "octree.glsl"
#define CAMERA_SET 1
#include "camera.glsl"
#define ENVIRONMENT_MAP_SET 2
#define ENVIRONMENT_MAP_ENABLE_SAMPLE 0
#include "environment_map.glsl"
layout(set = 3, binding = 0) uniform sampler2D uBeamImage;
layout(location = 0) out vec4 oColor;

layout(push_constant) uniform uuPushConstant {
	uint uWidth, uHeight, uViewType, uLightType, uBeamEnable, uBeamSize;
	float uConstColor[3], uEnvMapRotation;
};

vec3 Heat(in float x) { return sin(clamp(x, 0.0, 1.0) * 3.0 - vec3(1, 2, 3)) * 0.5 + 0.5; }

vec3 Light(in vec3 d) {
	return uLightType == 0 ? vec3(uConstColor[0], uConstColor[1], uConstColor[2]) : EnvMap_Radiance(d, uEnvMapRotation);
}

void main() {
	vec3 o = uPosition.xyz, d = Camera_GenRay(ivec2(gl_FragCoord.xy) / vec2(uWidth, uHeight));

	float beam;
	if (uBeamEnable == 1) {
		ivec2 beam_coord = ivec2(gl_FragCoord.xy / uBeamSize);
		beam = min(min(texelFetch(uBeamImage, beam_coord, 0).r, texelFetch(uBeamImage, beam_coord + ivec2(1, 0), 0).r),
		           min(texelFetch(uBeamImage, beam_coord + ivec2(0, 1), 0).r,
		               texelFetch(uBeamImage, beam_coord + ivec2(1, 1), 0).r));
		o += d * beam;
	}

	vec3 pos, color, normal;
	uint iter;
	bool hit = Octree_RayMarchLeaf(o, d, pos, color, normal, iter);
	if (!hit) {
		pos = vec3(1.0);
		normal = vec3(0.0);
		color = Light(d);
	}
	oColor =
	    vec4(uViewType == 3
	             ? Heat(iter / 128.0)
	             : (uViewType == 2 ? pos - 1.0 : (uViewType == 0 ? pow(color, vec3(1.0 / 2.2)) : normal * 0.5 + 0.5)),
	         1.0);
}

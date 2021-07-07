#version 450

layout(rgba32f, set = 0, binding = 0) uniform image2D uColor;
layout(rgba8, set = 0, binding = 1) uniform image2D uAlbedo;
layout(rgba8_snorm, set = 0, binding = 2) uniform image2D uNormal;

layout(location = 0) out vec4 oColor;

layout(push_constant) uniform uuPushConstant { uint uViewType; };

void main() {
	if (uViewType == 0) {
		vec4 color = imageLoad(uColor, ivec2(gl_FragCoord.xy));
		oColor = vec4(pow(color.xyz, vec3(1.0 / 2.2)), 1.0);
	} else if (uViewType == 1) {
		vec4 albedo = imageLoad(uAlbedo, ivec2(gl_FragCoord.xy));
		oColor = vec4(albedo.xyz, 1.0);
	} else {
		vec4 normal = imageLoad(uNormal, ivec2(gl_FragCoord.xy));
		oColor = vec4(normal.xyz * 0.5 + 0.5, 1.0);
	}
}

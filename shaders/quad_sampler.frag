#version 450
layout(binding = 0) uniform sampler2D uSampler;
layout(location = 0) out vec4 oColor;

void main() {
	oColor = vec4(texelFetch(uSampler, ivec2(gl_FragCoord.xy), 0).xyz, 1.0);
}

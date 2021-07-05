#version 450
layout(binding = 0) uniform sampler2D uSampler;
layout(location = 0) out vec4 oColor;
layout(location = 0) in vec2 vTexcoord;

void main() { oColor = vec4(texture(uSampler, vTexcoord).xyz, 1.0); }

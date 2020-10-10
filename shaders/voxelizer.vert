#version 450

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexcoord;

layout(location = 0) out vec2 vTexcoord;

void main() {
	gl_Position = vec4(aPosition, 1.0f);
	vTexcoord = aTexcoord;
}

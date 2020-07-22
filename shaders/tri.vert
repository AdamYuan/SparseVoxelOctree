#version 450

layout (set = 0, binding = 0) uniform uuCamera {
	mat4 uProjection;
	mat4 uView;
};

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexcoord;

layout (location = 0) out vec3 vNormal;
layout (location = 1) out vec2 vTexcoord;

void main() {
	gl_Position = uProjection * uView * vec4(aPosition, 1.0);
	vNormal = aNormal;
	vTexcoord = aTexcoord;
}

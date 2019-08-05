#version 450 core
layout (location = 0) in vec2 aPosition;
layout(std140, binding = 5) uniform uuCamera
{
	int uWidth, uHeight, uBeamSize, uA;
	mat4 uProjection;
	mat4 uView;
	vec4 uPosition;
};
void main() { gl_Position = vec4(aPosition, 1.0, 1.0); }

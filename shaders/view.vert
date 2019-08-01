#version 450 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexcoords;
layout (location = 3) in int aTexture;
layout (location = 4) in vec3 aDiffColor;

out vec2 vTexcoords;
out vec3 vFragPos, vNormal;
flat out int vTexture;
flat out vec3 vDiffColor;

layout(std140, binding = 5) uniform uuCamera
{
	mat4 uProjection;
	mat4 uView;
	vec4 uPosition;
};

void main()
{
	vFragPos = aPosition;
	vTexcoords = aTexcoords;
	vNormal = aNormal;
    vDiffColor = aDiffColor;
    vTexture = aTexture;
	gl_Position = uProjection * uView * vec4(aPosition, 1.0f);
}

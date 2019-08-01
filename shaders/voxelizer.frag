#version 450 core
#extension GL_ARB_bindless_texture : require

layout(binding = 0) uniform uuTextures { sampler2D uTextures[1024]; };
layout(binding = 1, offset = 0) uniform atomic_uint uCounter;
layout(std430, binding = 2) buffer uuFragmentList { uvec2 uFragmentList[]; };

in vec2 gTexcoords;
in vec3 gNormal;
in vec3 gVoxelPos;
in vec2 gScreenPos;
flat in int gTexture;
flat in vec3 gDiffColor;

uniform int uVoxelResolution, uCountOnly;

void main()
{
	vec4 samp = gTexture == -1 ? vec4(gDiffColor, 1.0) : texture(uTextures[gTexture], gTexcoords);
	if(samp.w < 0.5f) discard;

	uint cur = atomicCounterIncrement(uCounter);
	//set fragment list
	if(uCountOnly == 0)
	{
		uvec3 uvoxel_pos = clamp(uvec3(gVoxelPos), uvec3(0u), uvec3(uVoxelResolution - 1u));
		uFragmentList[cur].x = uvoxel_pos.x | (uvoxel_pos.y << 10u) | (uvoxel_pos.z << 20u);
		uFragmentList[cur].y = packUnorm4x8(samp);
	}
}

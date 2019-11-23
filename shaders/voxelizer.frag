#version 450 core
#extension GL_ARB_bindless_texture : require

layout(binding = 0) uniform uuTextures { sampler2D uTextures[1024]; };
layout(binding = 1, offset = 0) uniform atomic_uint uCounter;
layout(std430, binding = 2) buffer uuFragmentList { uvec2 uFragmentList[]; };

in vec2 gTexcoords;
in vec3 gNormal;
in vec3 gVoxelPos;
flat in uint gTexture;
flat in vec3 gColor;

uniform int uVoxelResolution, uCountOnly;

void main()
{
	vec4 samp = gTexture != 0xffffffffu ? texture(uTextures[gTexture], gTexcoords) : vec4(gColor, 1.0);
	if(samp.w < 0.5f) discard;

	uvec3 ucolor = uvec3(round(samp.rgb * 255.0f));
	if(ucolor == uvec3(0u))
		ucolor = uvec3(16u);
	ucolor &= 0xff;

	uint cur = atomicCounterIncrement(uCounter);
	//set fragment list
	if(uCountOnly == 0)
	{
		uvec3 uvoxel_pos = clamp(uvec3(gVoxelPos), uvec3(0u), uvec3(uVoxelResolution - 1u));
		uFragmentList[cur].x = uvoxel_pos.x | (uvoxel_pos.y << 12u) | ((uvoxel_pos.z & 0xffu) << 24u); //only have the last 8 bits of uvoxel_pos.z
		uFragmentList[cur].y = ((uvoxel_pos.z >> 8u) << 28u) | ucolor.x | (ucolor.y << 8u) | (ucolor.z << 16u);
	}
}

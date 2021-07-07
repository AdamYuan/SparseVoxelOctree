#version 450

layout(constant_id = 0) const uint kTextureNum = 1024;

layout(std140, set = 0, binding = 0) buffer uuCounter { uint uCounter; };
layout(std430, set = 0, binding = 1) writeonly buffer uuFragmentList { uvec2 uFragmentList[]; };
layout(set = 1, binding = 0) uniform sampler2D uTextures[kTextureNum];

layout(push_constant) uniform uuPushConstant { uint uVoxelResolution, uCountOnly, uTextureId, uAlbedo; };

layout(location = 0) in vec2 gTexcoord;
layout(location = 1) in vec3 gVoxelPos;

vec4 SampleOrDiscard() {
	vec4 x = texture(uTextures[uTextureId], gTexcoord);
	if (x.a < 0.5f)
		discard;
	return x;
}

void main() {
	uint ucolor = (uTextureId == 0xffffffffu) ? uAlbedo : packUnorm4x8(SampleOrDiscard());
	uint cur = atomicAdd(uCounter, 1u);
	// set fragment list
	if (uCountOnly == 0) {
		uvec3 uvoxel_pos = clamp(uvec3(gVoxelPos * uVoxelResolution), uvec3(0u), uvec3(uVoxelResolution - 1u));
		uFragmentList[cur].x = uvoxel_pos.x | (uvoxel_pos.y << 12u) |
		                       ((uvoxel_pos.z & 0xffu) << 24u); // only have the last 8 bits of uvoxel_pos.z
		uFragmentList[cur].y = ((uvoxel_pos.z >> 8u) << 28u) | (ucolor & 0x00ffffffu);
	}
}

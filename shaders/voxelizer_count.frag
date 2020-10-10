#version 450

layout(constant_id = 0) const uint kTextureNum = 1024;

layout(std140, set = 0, binding = 0) buffer uuCounter { uint uCounter; };
layout(set = 1, binding = 0) uniform sampler2D uTextures[kTextureNum];

layout(push_constant) uniform uuPushConstant {
	uint uVoxelResolution, uCountOnly, uTextureId, uAlbedo;
};

layout(location = 0) in vec2 gTexcoord;
layout(location = 1) in vec3 gVoxelPos;

vec4 SampleOrDiscard() {
	vec4 x = texture(uTextures[uTextureId], gTexcoord);
	if (x.a < 0.5f)
		discard;
	return x;
}

void main() {
	uint ucolor =
	    (uTextureId == 0xffffffffu) ? uAlbedo : packUnorm4x8(SampleOrDiscard());
	atomicAdd(uCounter, 1u);
}

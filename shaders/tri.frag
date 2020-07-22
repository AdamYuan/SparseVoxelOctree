#version 450

layout (location = 0) in vec3 vNormal;
layout (location = 1) in vec2 vTexcoord;

layout (location = 0) out vec4 oColor;

layout (set = 1, binding = 0) uniform sampler2D uTextures[1024];
layout(push_constant) uniform uuPushConstant { uint uTextureId, uAlbedo; };

vec3 SampleOrDiscard() {
	vec4 x = texture(uTextures[uTextureId], vTexcoord);
	if (x.a < 0.5) discard;
	return x.rgb;
}

void main() {
	vec3 albedo = (uTextureId == 0xffffffff) ? unpackUnorm4x8(uAlbedo).xyz : SampleOrDiscard();
	float shade = max(dot(normalize(vNormal), normalize(vec3(-4.0, 10.0, 3.0))), 0.0) * 0.5 + 0.5;
	oColor = vec4(pow(albedo * shade, vec3(1.0 / 2.2)), 1.0);
}

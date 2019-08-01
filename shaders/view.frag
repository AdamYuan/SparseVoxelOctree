#version 450 core
#extension GL_ARB_bindless_texture : require

layout(binding = 0) uniform uuTextures { sampler2D uTextures[1024]; };

out vec4 GAlbedo;

in vec2 vTexcoords;
in vec3 vFragPos, vNormal;
flat in int vTexture;
flat in vec3 vDiffColor;

void main()
{
	vec4 samp = vTexture == -1 ? vec4(vDiffColor, 1.0) : texture(uTextures[vTexture], vTexcoords);
	if(samp.a < .5f) discard;
	GAlbedo = vec4(samp.rgb, 1.0);
}

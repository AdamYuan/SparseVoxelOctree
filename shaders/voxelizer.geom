#version 450 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vNormal[];
in vec2 vTexcoords[];
flat in int vTexture[];
flat in vec3 vDiffColor[];

out vec2 gTexcoords;
out vec3 gNormal;
out vec3 gVoxelPos;
flat out int gTexture;
flat out vec3 gDiffColor;

uniform int uVoxelResolution;

vec3 Project(in vec3 v, in int axis) { return axis == 0 ? v.yzx : (axis == 1 ? v.xzy : v); }
vec3 Unproject(in vec3 v, in int axis) { return axis == 0 ? v.zxy : (axis == 1 ? v.xzy : v); }

void main()
{
	//get projection axis
	vec3 axis_weight = abs(cross(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz));
	int axis = 2;
	if(axis_weight.x >= axis_weight.y && axis_weight.x > axis_weight.z) axis = 0;
	else if(axis_weight.y >= axis_weight.z && axis_weight.y > axis_weight.x) axis = 1;

	float half_pixel = 1.0f / uVoxelResolution; //full pixel size is 2.0f / uVoxelResolution

	//project the positions
	vec3 pos0 = Project(gl_in[0].gl_Position.xyz, axis);
	vec3 pos1 = Project(gl_in[1].gl_Position.xyz, axis);
	vec3 pos2 = Project(gl_in[2].gl_Position.xyz, axis);

	gTexture = vTexture[0];
	gDiffColor = vDiffColor[0];

	gTexcoords = vTexcoords[0];
	gNormal = normalize(vNormal[0]);
	gVoxelPos = (Unproject(pos0, axis) + 1.0f) * 0.5f * uVoxelResolution;
	gl_Position = vec4(pos0.xy, 1.0f, 1.0f);
	EmitVertex();
	gTexcoords = vTexcoords[1];
	gNormal = normalize(vNormal[1]);
	gVoxelPos = (Unproject(pos1, axis) + 1.0f) * 0.5f * uVoxelResolution;
	gl_Position = vec4(pos1.xy, 1.0f, 1.0f);
	EmitVertex();
	gTexcoords = vTexcoords[2];
	gNormal = normalize(vNormal[2]);
	gVoxelPos = (Unproject(pos2, axis) + 1.0f) * 0.5f * uVoxelResolution;
	gl_Position = vec4(pos2.xy, 1.0f, 1.0f);
	EmitVertex();
	EndPrimitive();
}

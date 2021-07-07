#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec2 vTexcoord[];

layout(location = 0) out vec2 gTexcoord;
layout(location = 1) out vec3 gVoxelPos;

vec2 Project(in vec3 v, in uint axis) { return axis == 0 ? v.yz : (axis == 1 ? v.xz : v.xy); }

void main() {
	// project the positions
	vec3 pos0 = gl_in[0].gl_Position.xyz;
	vec3 pos1 = gl_in[1].gl_Position.xyz;
	vec3 pos2 = gl_in[2].gl_Position.xyz;

	// get projection axis
	vec3 axis_weight = abs(cross(pos1 - pos0, pos2 - pos0));
	uint axis = (axis_weight.x > axis_weight.y && axis_weight.x > axis_weight.z)
	                ? 0
	                : ((axis_weight.y > axis_weight.z) ? 1 : 2);

	gTexcoord = vTexcoord[0];
	gVoxelPos = (pos0 + 1.0f) * 0.5f;
	gl_Position = vec4(Project(pos0, axis), 1.0f, 1.0f);
	EmitVertex();
	gTexcoord = vTexcoord[1];
	gVoxelPos = (pos1 + 1.0f) * 0.5f;
	gl_Position = vec4(Project(pos1, axis), 1.0f, 1.0f);
	EmitVertex();
	gTexcoord = vTexcoord[2];
	gVoxelPos = (pos2 + 1.0f) * 0.5f;
	gl_Position = vec4(Project(pos2, axis), 1.0f, 1.0f);
	EmitVertex();
	EndPrimitive();
}

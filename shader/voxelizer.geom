#version 450

layout(constant_id = 0) const uint kVoxelResolution = 1;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec2 vTexcoord[];

layout(location = 0) out vec2 gTexcoord;
layout(location = 1) flat out uint gAxis;

vec3 Project(in vec3 v, in uint axis) {
	vec3 ret = axis == 0 ? v.yzx : (axis == 1 ? v.zxy : v.xyz);
	ret.z = (ret.z + 1.0) * 0.5;
	return ret;
}

void main() {
	// project the positions
	vec3 pos0 = gl_in[0].gl_Position.xyz;
	vec3 pos1 = gl_in[1].gl_Position.xyz;
	vec3 pos2 = gl_in[2].gl_Position.xyz;

	// get projection axis
	vec3 normal = cross(pos1 - pos0, pos2 - pos0);
	vec3 axis_weight = abs(normal);
	uint axis = (axis_weight.x > axis_weight.y && axis_weight.x > axis_weight.z)
	                ? 0
	                : ((axis_weight.y > axis_weight.z) ? 1 : 2);

	vec3 ndc0 = Project(pos0, axis);
	vec3 ndc1 = Project(pos1, axis);
	vec3 ndc2 = Project(pos2, axis);

	gAxis = axis;
	gTexcoord = vTexcoord[0];
	gl_Position = vec4(Project(pos0, axis), 1.0);
	EmitVertex();
	gTexcoord = vTexcoord[1];
	gl_Position = vec4(Project(pos1, axis), 1.0);
	EmitVertex();
	gTexcoord = vTexcoord[2];
	gl_Position = vec4(Project(pos2, axis), 1.0);
	EmitVertex();
	EndPrimitive();
}

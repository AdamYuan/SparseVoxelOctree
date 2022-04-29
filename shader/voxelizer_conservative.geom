#version 450

layout(constant_id = 0) const uint kVoxelResolution = 1;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec2 vTexcoord[];

layout(location = 0) out vec2 gTexcoord;
layout(location = 1) flat out uint gAxis;
layout(location = 2) flat out vec4 gAABB;

vec3 Project(in vec3 v, in uint axis) {
	vec3 ret = axis == 0 ? v.yzx : (axis == 1 ? v.zxy : v.xyz);
	ret.z = (ret.z + 1.0) * 0.5;
	return ret;
}

vec3 GetBarycentric(in const vec2 p, in const vec2 a, in const vec2 b, in const vec2 c) {
	float l0 = ((b.y - c.y) * (p.x - c.x) + (c.x - b.x) * (p.y - c.y)) /
	           ((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));
	float l1 = ((c.y - a.y) * (p.x - c.x) + (a.x - c.x) * (p.y - c.y)) /
	           ((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));
	return vec3(l0, l1, 1.0 - l0 - l1);
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

	vec3 line0 = cross(vec3(ndc2.xy, 1.0), vec3(ndc1.xy, 1.0));
	vec3 line1 = cross(vec3(ndc0.xy, 1.0), vec3(ndc2.xy, 1.0));
	vec3 line2 = cross(vec3(ndc1.xy, 1.0), vec3(ndc0.xy, 1.0));

	if (normal[axis] < 0.0) {
		line0.z += dot(vec2(1.0 / float(kVoxelResolution)), abs(line0.xy));
		line1.z += dot(vec2(1.0 / float(kVoxelResolution)), abs(line1.xy));
		line2.z += dot(vec2(1.0 / float(kVoxelResolution)), abs(line2.xy));
	} else {
		line0.z -= dot(vec2(1.0 / float(kVoxelResolution)), abs(line0.xy));
		line1.z -= dot(vec2(1.0 / float(kVoxelResolution)), abs(line1.xy));
		line2.z -= dot(vec2(1.0 / float(kVoxelResolution)), abs(line2.xy));
	}

	vec3 intersect0 = cross(line2, line1);
	vec3 intersect1 = cross(line0, line2);
	vec3 intersect2 = cross(line1, line0);
	intersect0.xy /= intersect0.z;
	intersect1.xy /= intersect1.z;
	intersect2.xy /= intersect2.z;

	vec3 bc0 = GetBarycentric(intersect0.xy, ndc0.xy, ndc1.xy, ndc2.xy);
	vec3 bc1 = GetBarycentric(intersect1.xy, ndc0.xy, ndc1.xy, ndc2.xy);
	vec3 bc2 = GetBarycentric(intersect2.xy, ndc0.xy, ndc1.xy, ndc2.xy);

	gAxis = axis;
	gAABB = vec4((min(ndc0.xy, min(ndc1.xy, ndc2.xy)) + 1.0) * 0.5 * kVoxelResolution - 0.5,
	             (max(ndc0.xy, max(ndc1.xy, ndc2.xy)) + 1.0) * 0.5 * kVoxelResolution + 0.5);

	vec3 z_trans = vec3(ndc0.z, ndc1.z, ndc2.z);
	mat3x2 texcoords_trans = mat3x2(vTexcoord[0], vTexcoord[1], vTexcoord[2]);

	gTexcoord = texcoords_trans * bc0;
	gl_Position = vec4(intersect0.xy, dot(bc0, z_trans), 1.0);
	EmitVertex();
	gTexcoord = texcoords_trans * bc1;
	gl_Position = vec4(intersect1.xy, dot(bc1, z_trans), 1.0);
	EmitVertex();
	gTexcoord = texcoords_trans * bc2;
	gl_Position = vec4(intersect2.xy, dot(bc2, z_trans), 1.0);
	EmitVertex();
	EndPrimitive();
}

#version 450

layout(location = 0) out vec2 vTexcoord;

layout(push_constant) uniform uuPushConstant {
	vec2 uTexcoords[3];
};

void main() {
	gl_Position =
	    vec4(vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2) * 2.0 - 1.0,
	         0.0, 1.0);
	vTexcoord = uTexcoords[gl_VertexIndex];
}

#version 450

layout(location = 0) in vec2 aTexcoord;

layout(location = 0) out vec2 vTexcoord;

void main() {
	gl_Position = vec4(vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2) * 2.0 - 1.0, 0.0, 1.0);
	vTexcoord = aTexcoord;
}

#ifndef SOBOL_GLSL
#define SOBOL_GLSL

#ifndef SOBOL_SET
#define SOBOL_SET 3
#endif

layout(std430, set = SOBOL_SET, binding = 0) buffer uuSobol { uint uSobol[]; };
vec2 Sobol_GetVec2(in const uint i) { return vec2(uSobol[i << 1u], uSobol[i << 1u | 1u]) / 4294967296.0; }
uint Sobol_Current(in const uint dim) { return uSobol[dim]; }

#endif

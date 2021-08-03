#version 450
layout(std430, set = 0, binding = 0) readonly buffer uuOctree { uint uOctree[]; };
layout(set = 1, binding = 0) uniform uuCamera { vec4 uPosition, uLook, uSide, uUp; };
layout(set = 2, binding = 0) uniform sampler2D uEnvironmentMap;
layout(set = 3, binding = 0) uniform sampler2D uBeamImage;
layout(location = 0) out vec4 oColor;

layout(push_constant) uniform uuPushConstant {
	uint uWidth, uHeight, uViewType, uLightType, uBeamEnable, uBeamSize;
	float uConstColor[3], uEnvMapRotation;
};

bool RayMarchLeaf(vec3 o, vec3 d, out float o_t, out vec3 o_color, out vec3 o_normal, out uint o_iter);

vec3 GenRay() {
	vec2 coord = ivec2(gl_FragCoord.xy) / vec2(uWidth, uHeight);
	coord = coord * 2.0f - 1.0f;
	return normalize(uLook.xyz - uSide.xyz * coord.x - uUp.xyz * coord.y);
}

vec3 Heat(in float x) { return sin(clamp(x, 0.0, 1.0) * 3.0 - vec3(1, 2, 3)) * 0.5 + 0.5; }

#define PI 3.1415926535897932384626433832795
vec2 EnvMapCoord(in vec3 d) { return vec2((atan(d.x, d.z) + uEnvMapRotation) / PI * 0.5, acos(d.y) / PI); }

vec3 Light(in vec3 d) {
	return uLightType == 0 ? vec3(uConstColor[0], uConstColor[1], uConstColor[2])
	                       : texture(uEnvironmentMap, EnvMapCoord(d)).xyz;
}

void main() {
	vec3 o = uPosition.xyz, d = GenRay();

	float beam;
	if (uBeamEnable == 1) {
		ivec2 beam_coord = ivec2(gl_FragCoord.xy / uBeamSize);
		beam = min(min(texelFetch(uBeamImage, beam_coord, 0).r, texelFetch(uBeamImage, beam_coord + ivec2(1, 0), 0).r),
		           min(texelFetch(uBeamImage, beam_coord + ivec2(0, 1), 0).r,
		               texelFetch(uBeamImage, beam_coord + ivec2(1, 1), 0).r));
		o += d * beam;
	}

	float t;
	vec3 color, normal;
	uint iter;
	bool hit = RayMarchLeaf(o, d, t, color, normal, iter);
	if (!hit) {
		normal = vec3(0.0);
		color = Light(d);
	}
	oColor = vec4(
	    uViewType == 2 ? Heat(iter / 128.0) : (uViewType == 0 ? pow(color, vec3(1.0 / 2.2)) : normal * 0.5 + 0.5), 1.0);
}

// The following code is copied from
// https://code.google.com/archive/p/efficient-sparse-voxel-octrees/
/*
 *  Copyright (c) 2009-2011, NVIDIA Corporation
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of NVIDIA Corporation nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define STACK_SIZE 23
#define EPS 3.552713678800501e-15
struct StackItem {
	uint node;
	float t_max;
} stack[STACK_SIZE];
bool RayMarchLeaf(vec3 o, vec3 d, out float o_t, out vec3 o_color, out vec3 o_normal, out uint o_iter) {
	uint iter = 0;

	d.x = abs(d.x) > EPS ? d.x : (d.x >= 0 ? EPS : -EPS);
	d.y = abs(d.y) > EPS ? d.y : (d.y >= 0 ? EPS : -EPS);
	d.z = abs(d.z) > EPS ? d.z : (d.z >= 0 ? EPS : -EPS);

	// Precompute the coefficients of tx(x), ty(y), and tz(z).
	// The octree is assumed to reside at coordinates [1, 2].
	vec3 t_coef = 1.0f / -abs(d);
	vec3 t_bias = t_coef * o;

	uint oct_mask = 0u;
	if (d.x > 0.0f)
		oct_mask ^= 1u, t_bias.x = 3.0f * t_coef.x - t_bias.x;
	if (d.y > 0.0f)
		oct_mask ^= 2u, t_bias.y = 3.0f * t_coef.y - t_bias.y;
	if (d.z > 0.0f)
		oct_mask ^= 4u, t_bias.z = 3.0f * t_coef.z - t_bias.z;

	// Initialize the active span of t-values.
	float t_min = max(max(2.0f * t_coef.x - t_bias.x, 2.0f * t_coef.y - t_bias.y), 2.0f * t_coef.z - t_bias.z);
	float t_max = min(min(t_coef.x - t_bias.x, t_coef.y - t_bias.y), t_coef.z - t_bias.z);
	t_min = max(t_min, 0.0f);
	float h = t_max;

	uint parent = 0u;
	uint cur = 0u;
	vec3 pos = vec3(1.0f);
	uint idx = 0u;
	if (1.5f * t_coef.x - t_bias.x > t_min)
		idx ^= 1u, pos.x = 1.5f;
	if (1.5f * t_coef.y - t_bias.y > t_min)
		idx ^= 2u, pos.y = 1.5f;
	if (1.5f * t_coef.z - t_bias.z > t_min)
		idx ^= 4u, pos.z = 1.5f;

	uint scale = STACK_SIZE - 1;
	float scale_exp2 = 0.5f; // exp2( scale - STACK_SIZE )

	while (scale < STACK_SIZE) {
		++iter;
		if (cur == 0u)
			cur = uOctree[parent + (idx ^ oct_mask)];
		// Determine maximum t-value of the cube by evaluating
		// tx(), ty(), and tz() at its corner.

		vec3 t_corner = pos * t_coef - t_bias;
		float tc_max = min(min(t_corner.x, t_corner.y), t_corner.z);

		if ((cur & 0x80000000u) != 0 && t_min <= t_max) {
			// INTERSECT
			float tv_max = min(t_max, tc_max);
			float half_scale_exp2 = scale_exp2 * 0.5f;
			vec3 t_center = half_scale_exp2 * t_coef + t_corner;

			if (t_min <= tv_max) {
				if ((cur & 0x40000000u) != 0) // leaf node
					break;

				// PUSH
				if (tc_max < h) {
					stack[scale].node = parent;
					stack[scale].t_max = t_max;
				}
				h = tc_max;

				parent = cur & 0x3fffffffu;

				idx = 0u;
				--scale;
				scale_exp2 = half_scale_exp2;
				if (t_center.x > t_min)
					idx ^= 1u, pos.x += scale_exp2;
				if (t_center.y > t_min)
					idx ^= 2u, pos.y += scale_exp2;
				if (t_center.z > t_min)
					idx ^= 4u, pos.z += scale_exp2;

				cur = 0;
				t_max = tv_max;

				continue;
			}
		}

		// ADVANCE
		uint step_mask = 0u;
		if (t_corner.x <= tc_max)
			step_mask ^= 1u, pos.x -= scale_exp2;
		if (t_corner.y <= tc_max)
			step_mask ^= 2u, pos.y -= scale_exp2;
		if (t_corner.z <= tc_max)
			step_mask ^= 4u, pos.z -= scale_exp2;

		// Update active t-span and flip bits of the child slot index.
		t_min = tc_max;
		idx ^= step_mask;

		// Proceed with pop if the bit flips disagree with the ray direction.
		if ((idx & step_mask) != 0) {
			// POP
			// Find the highest differing bit between the two positions.
			uint differing_bits = 0;
			if ((step_mask & 1u) != 0)
				differing_bits |= floatBitsToUint(pos.x) ^ floatBitsToUint(pos.x + scale_exp2);
			if ((step_mask & 2u) != 0)
				differing_bits |= floatBitsToUint(pos.y) ^ floatBitsToUint(pos.y + scale_exp2);
			if ((step_mask & 4u) != 0)
				differing_bits |= floatBitsToUint(pos.z) ^ floatBitsToUint(pos.z + scale_exp2);
			scale = findMSB(differing_bits);
			scale_exp2 = uintBitsToFloat((scale - STACK_SIZE + 127u) << 23u); // exp2f(scale - s_max)

			// Restore parent voxel from the stack.
			parent = stack[scale].node;
			t_max = stack[scale].t_max;

			// Round cube position and extract child slot index.
			uint shx = floatBitsToUint(pos.x) >> scale;
			uint shy = floatBitsToUint(pos.y) >> scale;
			uint shz = floatBitsToUint(pos.z) >> scale;
			pos.x = uintBitsToFloat(shx << scale);
			pos.y = uintBitsToFloat(shy << scale);
			pos.z = uintBitsToFloat(shz << scale);
			idx = (shx & 1u) | ((shy & 1u) << 1u) | ((shz & 1u) << 2u);

			// Prevent same parent from being stored again and invalidate cached
			// child descriptor.
			h = 0.0f;
			cur = 0;
		}
	}

	vec3 t_corner = t_coef * (pos + scale_exp2) - t_bias;

	vec3 norm = (t_corner.x > t_corner.y && t_corner.x > t_corner.z)
	                ? vec3(-1, 0, 0)
	                : (t_corner.y > t_corner.z ? vec3(0, -1, 0) : vec3(0, 0, -1));
	if ((oct_mask & 1u) == 0u)
		norm.x = -norm.x;
	if ((oct_mask & 2u) == 0u)
		norm.y = -norm.y;
	if ((oct_mask & 4u) == 0u)
		norm.z = -norm.z;

	o_normal = norm;
	o_color = unpackUnorm4x8(cur).xyz;
	o_t = t_min;
	o_iter = iter;

	return scale < STACK_SIZE && t_min <= t_max;
}

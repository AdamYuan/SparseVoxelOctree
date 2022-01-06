#ifndef UTIL_GLSL
#define UTIL_GLSL

#define PI 3.1415926535897932384626433832795
vec3 AlignDirection(in const vec3 dir, in const vec3 target) {
	vec3 u = normalize(cross(abs(target.x) > .01 ? vec3(0, 1, 0) : vec3(1, 0, 0), target));
	vec3 v = cross(target, u);
	return dir.x * u + dir.y * v + dir.z * target;
}

#endif

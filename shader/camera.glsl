#ifndef CAMERA_HPP
#define CAMERA_HPP

#ifndef CAMERA_SET
#define CAMERA_SET 1
#endif

layout(set = CAMERA_SET, binding = 0) uniform uuCamera { vec4 uPosition, uLook, uSide, uUp; };

// coord [0, 1]
vec3 Camera_GenRay(vec2 coord) {
	coord = coord * 2.0f - 1.0f;
	return normalize(uLook.xyz - uSide.xyz * coord.x - uUp.xyz * coord.y);
}

#endif

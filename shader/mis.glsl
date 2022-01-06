#ifndef MIS_GLSL
#define MIS_GLSL

float MIS_PowerHeuristic(in const float a, in const float b) {
	float t = a * a;
	return t / (b * b + t);
}

#endif

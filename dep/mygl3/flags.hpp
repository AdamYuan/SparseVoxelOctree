//
// Created by adamyuan on 12/8/18.
//

#ifndef MYGL3_FLAGS_HPP
#define MYGL3_FLAGS_HPP

#include <GL/gl3w.h>
namespace mygl3
{
	constexpr GLuint kInvalidOglId = 0xffffffffu;
	static bool IsValidOglId(GLuint id) { return id != kInvalidOglId; }
}

#endif

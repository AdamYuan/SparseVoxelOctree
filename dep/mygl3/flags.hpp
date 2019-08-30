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
	inline static void SyncGPU()
	{
		GLsync sync_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		GLenum wait_return = GL_UNSIGNALED;
		while (wait_return != GL_ALREADY_SIGNALED && wait_return != GL_CONDITION_SATISFIED)
			wait_return = glClientWaitSync(sync_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		glDeleteSync(sync_fence);
	}
}

#endif

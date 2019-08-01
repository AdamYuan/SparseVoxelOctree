//
// Created by adamyuan on 19-5-10.
//

#ifndef ATOMICCOUNTER_HPP
#define ATOMICCOUNTER_HPP

#include "buffer.hpp"

namespace mygl3
{
	class AtomicCounter
	{
	private:
		Buffer buffer_;
		GLuint *mapped_{nullptr};

	public:
		void Initialize() {
			buffer_.Initialize();
			GLbitfield flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
			buffer_.Storage(sizeof(GLuint), flags);
			mapped_ = (GLuint *)glMapNamedBufferRange(buffer_.Get(), 0, sizeof(GLuint), flags);
			*mapped_ = 0u;
		}
		~AtomicCounter() { if(mapped_) glUnmapNamedBuffer(buffer_.Get()); }
		void BindAtomicCounter(GLuint index) const { glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, index, buffer_.Get()); }
		void BindShaderStorage(GLuint index) const { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buffer_.Get()); }
		void Sync() const {
			glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
			GLsync sync_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			GLenum wait_return = GL_UNSIGNALED;
			while (wait_return != GL_ALREADY_SIGNALED && wait_return != GL_CONDITION_SATISFIED)
				wait_return = glClientWaitSync(sync_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
			glDeleteSync(sync_fence);
		}
		GLuint &GetValue() { return *mapped_; }
		const GLuint &GetValue() const { return *mapped_; }
		GLuint &SyncAndGetValue() { Sync(); return *mapped_; }
		const GLuint &SyncAndGetValue() const { Sync(); return *mapped_; }
		void Reset() { Sync(); *mapped_ = 0; }
	};
}

#endif

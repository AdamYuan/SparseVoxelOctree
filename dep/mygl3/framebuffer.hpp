//
// Created by adamyuan on 4/15/18.
//

#ifndef MYGL3_FRAMEBUFFER_HPP
#define MYGL3_FRAMEBUFFER_HPP

#include "flags.hpp"
#include "texture.hpp"

namespace mygl3
{
	class RenderBuffer
	{
	private:
		GLuint rbo_{kInvalidOglId};
	public:
		RenderBuffer() = default;
		RenderBuffer (const RenderBuffer&) = delete;
		RenderBuffer& operator= (const RenderBuffer&) = delete;
		RenderBuffer(RenderBuffer &&buffer) noexcept : rbo_(buffer.rbo_) { buffer.rbo_ = kInvalidOglId; }
		/*RenderBuffer& operator= (RenderBuffer&& buffer) noexcept
		{
			rbo_ = buffer.rbo_; buffer.rbo_ = kInvalidOglId;
			return *this;
		}*/
		~RenderBuffer() { if(IsValidOglId(rbo_)) glDeleteRenderbuffers(1, &rbo_); }
		void Initialize() { glCreateRenderbuffers(1, &rbo_); }
		void Load(GLenum internal_format, GLsizei width, GLsizei height)
		{ glNamedRenderbufferStorage(rbo_, internal_format, width, height); }
		GLuint Get() const { return rbo_; }
	};

	class FrameBuffer
	{
	private:
		GLuint fbo_{kInvalidOglId};
	public:
		FrameBuffer() = default;
		FrameBuffer (const FrameBuffer&) = delete;
		FrameBuffer& operator= (const FrameBuffer&) = delete;
		FrameBuffer(FrameBuffer &&buffer) noexcept : fbo_(buffer.fbo_) { buffer.fbo_ = kInvalidOglId; }
		/*FrameBuffer& operator= (FrameBuffer&& buffer) noexcept
		{
			fbo_ = buffer.fbo_; buffer.fbo_ = kInvalidOglId;
			return *this;
		}*/
		~FrameBuffer() { if(IsValidOglId(fbo_)) glDeleteFramebuffers(1, &fbo_); }
		void Initialize() { glCreateFramebuffers(1, &fbo_); }
		void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, fbo_); }
		static void Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

		void AttachTexture2D(const Texture2D &texture, GLenum attachment, GLint level = 0)
		{ glNamedFramebufferTexture(fbo_, attachment, texture.Get(), level); }
		void AttachRenderbuffer(const RenderBuffer &rbo, GLenum attachment)
		{ glNamedFramebufferRenderbuffer(fbo_, attachment, GL_RENDERBUFFER, rbo.Get()); }
		GLuint Get() const { return fbo_; }
	};

	class FrameBufferBinder
	{
	public:
		FrameBufferBinder() = delete;
		FrameBufferBinder (const FrameBufferBinder&) = delete;
		FrameBufferBinder& operator= (const FrameBufferBinder&) = delete;
		explicit FrameBufferBinder(FrameBuffer &fbo) { fbo.Bind(); }
		/*FrameBufferBinder(FrameBuffer &fbo, GLsizei width, GLsizei height)
		{
			fbo.Bind();
			glViewport(0, 0, width, height);
		}
		FrameBufferBinder(FrameBuffer &fbo, GLint x, GLint y, GLsizei width, GLsizei height)
		{
			fbo.Bind();
			glViewport(x, y, width, height);
		}*/
		~FrameBufferBinder() { FrameBuffer::Unbind(); }
	};
}

#endif //MYGL3_FRAMEBUFFER_HPP

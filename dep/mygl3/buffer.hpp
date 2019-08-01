#ifndef MYGL3_BUFFER_HPP
#define MYGL3_BUFFER_HPP

#include "flags.hpp"

namespace mygl3
{
	class Buffer
	{
	private:
		GLuint id_{kInvalidOglId};
	public:
		Buffer() = default;
		Buffer (const Buffer&) = delete;
		Buffer& operator= (const Buffer&) = delete;
		Buffer(Buffer &&buffer) noexcept : id_(buffer.id_) { buffer.id_ = kInvalidOglId; }
		/*Buffer& operator= (Buffer&& buffer) noexcept
		{
			id_ = buffer.id_; buffer.id_ = kInvalidOglId;
			return *this;
		}*/
		~Buffer() { if(IsValidOglId(id_)) glDeleteBuffers(1, &id_); }
		void Initialize() { glCreateBuffers(1, &id_); }
		GLuint Get() const { return id_; }
		void Storage(GLsizei bytes, GLbitfield flags) { glNamedBufferStorage(id_, bytes, nullptr, flags); }
		template<class T>
		void Storage(T *begin, T *end, GLbitfield flags)
		{ glNamedBufferStorage(id_, (end - begin)*sizeof(T), begin, flags); }

		//never use Data() with Storage()
		template<class T>
		void Data(T *begin, T *end, GLenum usage)
		{ glNamedBufferData(id_, (end - begin)*sizeof(T), begin, usage); }

		template<class T>
		void SubData(GLsizei byte_offset, const T *begin, const T *end)
		{ glNamedBufferSubData(id_, byte_offset, (end - begin) * sizeof(T), begin); }

		GLint GetByteCount() const
		{
			GLint ret;
			glGetNamedBufferParameteriv(id_, GL_BUFFER_SIZE, &ret);
			return ret;
		}

		void Bind(GLenum target) const { glBindBuffer(target, id_); }
		void BindBase(GLenum target, GLuint index) const { glBindBufferBase(target, index, id_); }
	};
}

#endif

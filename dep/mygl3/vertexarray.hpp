//
// Created by adamyuan on 3/10/18.
//

#ifndef MYGL3_VERTEXARRAY_HPP
#define MYGL3_VERTEXARRAY_HPP

#include "flags.hpp"
#include "buffer.hpp"

namespace mygl3
{
	struct VertexArrayAttrib { GLuint id; GLsizei bytes; GLenum type; };
	class VertexArray
	{
	private:
		GLuint id_{kInvalidOglId};
		GLsizei attr_tot_bytes_{0};
		const Buffer *vbo_ptr_{}, *ebo_ptr_{};
	public:
		VertexArray() = default;
		VertexArray(VertexArray &&buffer) noexcept :
				id_{buffer.id_}, attr_tot_bytes_{buffer.attr_tot_bytes_}, vbo_ptr_{buffer.vbo_ptr_}, ebo_ptr_{buffer.ebo_ptr_}
		{ buffer.id_ = kInvalidOglId; }
		~VertexArray() { if(IsValidOglId(id_)) glDeleteVertexArrays(1, &id_); }
		VertexArray (const VertexArray&) = delete;
		VertexArray& operator= (const VertexArray&) = delete;
		/*VertexArray &operator=(VertexArray &&buffer) noexcept
		{
			id_ = buffer.id_;
			attr_tot_bytes_ = buffer.attr_tot_bytes_;
			ebo_ptr_ = buffer.ebo_ptr_;
			vbo_ptr_ = buffer.vbo_ptr_;
			buffer.id_ = 0;
			return *this;
		}*/

		void Initialize() { glCreateVertexArrays(1, &id_); }
		GLuint Get() const { return id_; }

		void AddAttrib(GLuint id, GLsizei count, GLenum type, GLsizei sizeof_type)
		{
			glEnableVertexArrayAttrib(id_, id);
			glVertexArrayAttribFormat(id_, id, count, type, GL_FALSE, attr_tot_bytes_);
			glVertexArrayAttribBinding(id_, id, 0);
			attr_tot_bytes_ += count * sizeof_type;
		}
		void AddAttribI(GLuint id, GLsizei count, GLenum type, GLsizei sizeof_type)
		{
			glEnableVertexArrayAttrib(id_, id);
			glVertexArrayAttribIFormat(id_, id, count, type, attr_tot_bytes_);
			glVertexArrayAttribBinding(id_, id, 0);
			attr_tot_bytes_ += count * sizeof_type;
		}

		void BindVertexBuffer(const Buffer &vbo)
		{
			glVertexArrayVertexBuffer(id_, 0, vbo.Get(), 0, attr_tot_bytes_);
			vbo_ptr_ = &vbo;
		}

		void BindElementBuffer(const Buffer &ebo)
		{
			glVertexArrayElementBuffer(id_, ebo.Get());
			ebo_ptr_ = &ebo;
		}

		void DrawArrays(GLenum type) const
		{
			glBindVertexArray(id_);
			glDrawArrays(type, 0, vbo_ptr_->GetByteCount() / attr_tot_bytes_);
		}

		void DrawElements(GLenum type) const
		{
			glBindVertexArray(id_);
			glDrawElements(type, ebo_ptr_->GetByteCount() / sizeof(GLuint), GL_UNSIGNED_INT, nullptr);
		}

		void Bind() const { glBindVertexArray(id_); }
	};
}

#endif

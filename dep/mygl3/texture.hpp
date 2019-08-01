//
// Created by adamyuan on 4/14/18.
//

#ifndef MYGL3_TEXTURE_HPP
#define MYGL3_TEXTURE_HPP

#include "flags.hpp"

namespace mygl3
{
	//deleted mipmap
#define DEF_TEXTURE_CLASS(NAME, TARGET) class NAME { \
private: \
GLuint id_{kInvalidOglId}; \
public: \
NAME() = default; \
NAME& operator= (const NAME&) = delete; \
NAME(NAME &&texture) noexcept : id_(texture.id_) { texture.id_ = kInvalidOglId; } \
~NAME() { if(IsValidOglId(id_)) glDeleteTextures(1, &id_); } \
NAME (const NAME&) = delete; \
 void Initialize() { glCreateTextures(TARGET, 1, &id_); } \
 void Bind(GLuint unit) const { glBindTextureUnit(unit, id_); } \
 void SetSizeFilter(GLenum min_filter, GLenum mag_filter) { \
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, min_filter); \
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, mag_filter); \
} \
 void SetWrapFilter(GLenum filter) { \
	glTextureParameteri(id_, GL_TEXTURE_WRAP_S, filter); \
	glTextureParameteri(id_, GL_TEXTURE_WRAP_T, filter); \
	glTextureParameteri(id_, GL_TEXTURE_WRAP_R, filter); \
} \
GLuint Get() const { return id_; }

	DEF_TEXTURE_CLASS(Texture2D, GL_TEXTURE_2D)
		static  GLsizei GetLevelCount(GLsizei width, GLsizei height)
		{
			GLsizei cnt = 1;
			while((width | height) >> cnt) ++cnt;
			return cnt;
		}
		void Storage(GLsizei width, GLsizei height, GLenum internal_format, GLsizei levels = 1)
		{ glTextureStorage2D(id_, levels, internal_format, width, height); }
		void Data(const GLvoid *pixels, GLsizei width, GLsizei height, GLenum format, GLenum type, GLint level = 0)
		{ glTextureSubImage2D(id_, level, 0, 0, width, height, format, type, pixels); }
		void GenerateMipmap()
		{ glGenerateTextureMipmap(id_); }
	};
}

#endif //MYGL3_TEXTURE_HPP

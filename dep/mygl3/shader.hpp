//
// Created by adamyuan on 3/11/18.
//

#ifndef MYGL3_SHADER_HPP
#define MYGL3_SHADER_HPP

#include "flags.hpp"
#include <vector>
#include <fstream>

namespace mygl3
{
	class Shader
	{
	private:
		GLuint id_{kInvalidOglId};
		std::string load_file(const char *filename) const
		{
			std::ifstream in(filename);
			if(!in.is_open())
			{
				printf("failed to open file %s\n", filename);
				return "";
			}
			return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>{}};
		}
		std::vector<GLuint> shaders;

	public:
		Shader() = default;;
		~Shader()
		{
			for(GLuint i : shaders)
				glDeleteShader(i);
			shaders.clear();
			if(IsValidOglId(id_)) glDeleteProgram(id_);
		}
		Shader(const Shader&) = delete;
		Shader& operator= (const Shader&) = delete;
		Shader(Shader &&shader) noexcept : id_(shader.id_) { shader.id_ = 0; }
		/*Shader &operator=(Shader &&shader) noexcept
		{
			program_ = shader.program_;
			shader.program_ = 0;
			return *this;
		}*/

		void Initialize() { id_ = glCreateProgram(); }

		void Load(const char *src, GLenum type)
		{
			shaders.emplace_back(); GLuint &cur = shaders.back();

			cur = glCreateShader(type);
			glShaderSource(cur, 1, &src, nullptr);
			glCompileShader(cur);

			int success;
			glGetShaderiv(cur, GL_COMPILE_STATUS, &success);
			if(!success)
			{
				char log[16384];
				glGetShaderInfoLog(cur, 16384, nullptr, log);
				printf("******SHADER COMPILE ERROR******\nsrc:\n%s\nerr:\n%s\n\n\n", src, log);
			}
			glAttachShader(id_, cur);
		}

		void Finalize()
		{
			glLinkProgram(id_);
			for(GLuint i : shaders)
				glDeleteShader(i);
			shaders.clear();
		}

		void LoadFromFile(const char *filename, GLenum type)
		{
			std::string str {load_file(filename)};
			Load(str.c_str(), type);
		}

		void Use() const { if(!shaders.empty()) printf("ERROR\n"); glUseProgram(id_); }
		GLint GetUniform(const char *name) const
		{ return glGetUniformLocation(id_, name); }
		GLuint GetProgram() const { return id_; }

		void SetMat4(GLint loc, const GLfloat *matrix4)
		{ glProgramUniformMatrix4fv(id_, loc, 1, GL_FALSE, matrix4); }
		void SetMat3(GLint loc, const GLfloat *matrix3)
		{ glProgramUniformMatrix3fv(id_, loc, 1, GL_FALSE, matrix3); }
		void SetIVec4(GLint loc, const GLint *vector4)
		{ glProgramUniform4iv(id_, loc, 1, vector4); }
		void SetIVec3(GLint loc, const GLint *vector3)
		{ glProgramUniform3iv(id_, loc, 1, vector3); }
		void SetIVec2(GLint loc, const GLint *vector2)
		{ glProgramUniform2iv(id_, loc, 1, vector2); }
		void SetVec4(GLint loc, const GLfloat *vector4)
		{ glProgramUniform4fv(id_, loc, 1, vector4); }
		void SetVec3(GLint loc, const GLfloat *vector3)
		{ glProgramUniform3fv(id_, loc, 1, vector3); }
		void SetVec2(GLint loc, const GLfloat *vector2)
		{ glProgramUniform2fv(id_, loc, 1, vector2); }
		void SetInt(GLint loc, GLint i)
		{ glProgramUniform1i(id_, loc, i); }
		void SetUint(GLint loc, GLuint i)
		{ glProgramUniform1ui(id_, loc, i); }
		void SetFloat(GLint loc, GLfloat f)
		{ glProgramUniform1f(id_, loc, f); }
	};
}

#endif //MYGL3_SHADER_HPP

#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h> // include glad to get the required OpenGL headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace f3d {

	struct shader {
		// the program ID
		unsigned int ID;

		// constructor reads and builds the shader
		shader(const char* vertexPath, const char* fragmentPath) {

			// 1. retrieve the vertex/fragment source code from filePath
			std::string vertexCode;
			std::string fragmentCode;
			std::ifstream vShaderFile;
			std::ifstream fShaderFile;

			// ensure ifstream objects can throw exceptions:
			vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			try
			{
				// open files
				vShaderFile.open(vertexPath);
				fShaderFile.open(fragmentPath);
				std::stringstream vShaderStream, fShaderStream;
				// read file's buffer contents into streams
				vShaderStream << vShaderFile.rdbuf();
				fShaderStream << fShaderFile.rdbuf();
				// close file handlers
				vShaderFile.close();
				fShaderFile.close();
				// convert stream into string
				vertexCode = vShaderStream.str();
				fragmentCode = fShaderStream.str();
			}
			catch (std::ifstream::failure e)
			{
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
			}
			const char* vShaderCode = vertexCode.c_str();
			const char* fShaderCode = fragmentCode.c_str();

			// 2. compile shaders
			unsigned int vertex, fragment;
			int success;
			char infoLog[512];

			// vertex Shader
			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vShaderCode, NULL);
			glCompileShader(vertex);
			// print compile errors if any
			glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vertex, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
			};

			// similiar for Fragment Shader
			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &fShaderCode, NULL);
			glCompileShader(fragment);
			// print compile errors if any
			glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fragment, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
			};

			// shader Program
			ID = glCreateProgram();
			glAttachShader(ID, vertex);
			glAttachShader(ID, fragment);
			glLinkProgram(ID);
			// print linking errors if any
			glGetProgramiv(ID, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(ID, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" <<
					infoLog << std::endl;
			}
			// delete shaders; they�re linked into our program and no longer necessary
			glDeleteShader(vertex);
			glDeleteShader(fragment);
		}

		// use/activate the shader
		void use() {
			glUseProgram(ID);
		}

		int getUniformId(const std::string& name) {
			return glGetUniformLocation(ID, name.c_str());
		}

		// utility uniform functions
		void setUniform(const char* name, bool value) const {
			glUniform1i(glGetUniformLocation(ID, name), (int)value);
		}

		void setUniform(const char* name, int value) const {
			glUniform1i(glGetUniformLocation(ID, name), value);
		}

		void setUniform(const char* name, float value) const {
			glUniform1f(glGetUniformLocation(ID, name), value);
		}

		void setUniform(const char* name, const glm::mat4& value) const {
			glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, glm::value_ptr(value));
		}

		void setUniform(const char* name, const glm::mat3& value) const {
			glUniformMatrix3fv(glGetUniformLocation(ID, name), 1, GL_FALSE, glm::value_ptr(value));
		}

		void setUniform(const char* name, const glm::vec4& value) const {
			glUniform4fv(glGetUniformLocation(ID, name), 1, glm::value_ptr(value));
		}

		void setUniform(const char* name, const glm::vec3& value) const {
			glUniform3fv(glGetUniformLocation(ID, name), 1, glm::value_ptr(value));
		}

		void setUniform(int UID, bool value) const {
			glUniform1i(UID, (int)value);
		}

		void setUniform(int UID, int value) const {
			glUniform1i(UID, value);
		}

		void setUniform(int UID, float value) const {
			glUniform1f(UID, value);
		}

		void setUniform(int UID, const glm::mat4& value) const {
			glUniformMatrix4fv(UID, 1, GL_FALSE, glm::value_ptr(value));
		}

		void setUniform(int UID, const glm::mat3& value) const {
			glUniformMatrix3fv(UID, 1, GL_FALSE, glm::value_ptr(value));
		}

		void setUniform(int UID, const glm::vec4& value) const {
			glUniform4fv(UID, 1, glm::value_ptr(value));
		}

		void setUniform(int UID, const glm::vec3& value) const {
			glUniform3fv(UID, 1, glm::value_ptr(value));
		}

	};

}

#endif

#include <fstream>
#include <sstream>
#include <filesystem>

#include <iostream>

#include "OpenGL/Engine/Utils/OpenGLUtils.hpp"
#include "Utils/MirielEngineLogger.hpp"
#include "CustomErrors/MirielEngineErrors.hpp"

namespace MirielEngine::OpenGL {
	GLuint createShaderProgram(const char* vert, const char* frag) {
		GLuint vertShader = createShader(vert, GL_VERTEX_SHADER);
		GLuint fragShader = createShader(frag, GL_FRAGMENT_SHADER);

		GLuint shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertShader);
		glAttachShader(shaderProgram, fragShader);
		glLinkProgram(shaderProgram);

		int success;
		char infoLog[512];
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

		if (!success) {
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			throw MirielEngine::Errors::OpenGLUtilError(infoLog);
		}

		glDeleteShader(vertShader);
		glDeleteShader(fragShader);

		return shaderProgram;
	}

	GLuint createShader(const char* shaderName, GLenum type) {
		std::string shaderDir = shaderName;
		std::ifstream shaderFile(shaderDir, std::ios::in);

		if (!shaderFile.is_open() || shaderFile.bad()) {
			std::ostringstream os;
			os << "Failed Opening " << shaderName << " Shader File.";
			throw MirielEngine::Errors::OpenGLUtilError(os.str().c_str());
		}

		std::stringstream shaderCode;
		shaderCode << shaderFile.rdbuf();
		shaderFile.close();

		GLuint shader = glCreateShader(type);
		std::string strShaderCode = shaderCode.str();
		const char* convShaderCode = strShaderCode.c_str();
		glShaderSource(shader, 1, &convShaderCode, NULL);
		glCompileShader(shader);

		int success;
		char infoLog[512];
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (!success) {
			glGetShaderInfoLog(shader, 512, NULL, infoLog);
			throw MirielEngine::Errors::OpenGLUtilError(infoLog);
		}

		return shader;
	}
}
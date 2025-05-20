#pragma once

#include <string>

#include <glad/glad.h>

namespace MirielEngine::OpenGL {
	GLuint createShaderProgram(const char* vert, const char* frag);
	GLuint createShader(const char* shaderName, GLenum type);
}
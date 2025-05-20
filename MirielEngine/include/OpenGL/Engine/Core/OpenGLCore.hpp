#pragma once

#include <vector>
#include <string>

#include <glad/glad.h>

#include "Scenes/Objects.hpp"

namespace MirielEngine::OpenGL {
	class OpenGLCore {
		private:
			std::vector<GLuint> objectVBOs;
			std::vector<GLuint> objectVAOs;
			std::vector<GLuint> objectEBOs;
			std::unique_ptr<MirielEngine::Core::Scene> scene;
			std::vector<GLuint> particleVBOs;
			std::vector<GLuint> particleVAOs;
			std::vector<GLuint> textures;
			std::vector<GLuint> programs;
			size_t currentProgram;
		public:
			OpenGLCore();
			~OpenGLCore();
			void createBuffers();
			void createProgram();
			void nextProgram();
			void previousProgram();
			unsigned int loadTexture(const std::string& textureName);
			void draw(int width, int height);
	};
}
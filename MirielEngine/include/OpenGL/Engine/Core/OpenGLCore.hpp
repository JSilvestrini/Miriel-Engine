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
			std::vector<GLuint> UBOIDs;
			std::vector<GLuint> UBOs;
			std::vector<GLuint> particleVBOs;
			std::vector<GLuint> particleVAOs;
			std::vector<GLuint> textures;
			std::vector<GLuint> programs;
			std::shared_ptr<MirielEngine::Core::Scene> scene;
			size_t currentProgram;
		public:
			OpenGLCore();
			~OpenGLCore();
			void cleanUp();
			void createBuffers();
			void updateBuffers();
			void createProgram();
			void updateProgram();
			void nextProgram();
			void previousProgram();
			unsigned int loadTexture(const std::string& textureName);
			void draw(int width, int height);
			std::shared_ptr<MirielEngine::Core::Scene> getScene();
	};
}
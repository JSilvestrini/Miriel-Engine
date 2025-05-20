#pragma once

#include "glad/glad.h"
#include "Utils/MirielEngineLogger.hpp"
#include "Utils/MirielEngineWindow.hpp"
#include "OpenGL/Engine/Core/OpenGLCore.hpp"

namespace MirielEngine::OpenGL {
	class OpenGLApplication {
		private:
			GLFWwindow* window;
			std::unique_ptr<MirielEngine::OpenGL::OpenGLCore> core;
			int width;
			int height;
		public:
			OpenGLApplication();
			~OpenGLApplication() = default;
			void runApplication();
			void cleanup();
	};
}
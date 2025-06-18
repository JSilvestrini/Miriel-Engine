#pragma once

#include "glad/glad.h"
#include "Utils/MirielEngineLogger.hpp"
#include "Utils/MirielEngineWindow.hpp"
#include "OpenGL/Engine/Core/OpenGLCore.hpp"
#include "Utils/DearImGuiFrame.hpp"

namespace MirielEngine::OpenGL {
	class OpenGLApplication {
		private:
			std::string fontName;

			float highDPIScaleFactor;
			int fontSize;

			int width;
			int height;

			GLFWwindow* window;

			std::unique_ptr<MirielEngine::OpenGL::OpenGLCore> core;
			std::unique_ptr<MirielEngine::Utils::GUI> gui;

			void initializeDearImGUI();
		public:
			OpenGLApplication();
			~OpenGLApplication() = default;
			void runApplication();
			void cleanup();
	};
}
#include "OpenGL/OpenGLApplication.hpp"

namespace MirielEngine::OpenGL {

	OpenGLApplication::OpenGLApplication() {
		glfwInit();

		MirielEngine::Utils::GlobalLogger->log("OpenGL Application Launched.");

		window = NULL;

		try {
			window = MirielEngine::Core::createWindow(RENDER_BACKEND::OPENGL);
		} catch (MirielEngine::Errors::WindowCreationError& e) {
			glfwTerminate();
			throw MirielEngine::Errors::OpenGLError(e.what());
		}

		MirielEngine::Utils::GlobalLogger->log("Window Successfully Created.");
		MirielEngine::Utils::GlobalLogger->log("Making Context Current.");
		glfwMakeContextCurrent(window);

		MirielEngine::Utils::GlobalLogger->log("Attempting to Load Using GLAD");
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			throw MirielEngine::Errors::OpenGLError("Failed to Load Using GLAD.");
		}

		MirielEngine::Utils::GlobalLogger->log("Adding Window Callbacks.");
		MirielEngine::Core::setWindowCallbacks(window);
		MirielEngine::Utils::GlobalLogger->log("Setting Up OpenGL Core.");

		try {
			core = std::make_unique<MirielEngine::OpenGL::OpenGLCore>();
		} catch (const MirielEngine::Errors::OpenGLError&) {
			throw;
		}
		glfwGetWindowSize(window, &width, &height);
	}

	void OpenGLApplication::runApplication() {
		MirielEngine::Utils::GlobalLogger->log("Application has Started Main Loop.");
		/*
		* Each Function within this block will pretty much be within other files
		* Create Window + Callbacks will be within Utils
		* MirielEngine::Core::CreateWindow
		* MirielEngine::Core::SetCallbacks <- These will be within a Callback File
		*
		*/
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			core->draw(width, height);
			glfwSwapBuffers(window);
		}

		MirielEngine::Utils::GlobalLogger->log("Application has Exited Main Loop.");
	}

	void OpenGLApplication::cleanup() {
		MirielEngine::Utils::GlobalLogger->log("Closing Window.");
		MirielEngine::Utils::GlobalLogger->log("Destroying Window.");
		glfwDestroyWindow(window);
		MirielEngine::Utils::GlobalLogger->log("Terminating GLFW.");
		glfwTerminate();
	}
}
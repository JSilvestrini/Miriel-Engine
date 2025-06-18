#include "Utils/MirielEngineWindow.hpp"

#include "Utils/WindowCallbacks.hpp"

namespace MirielEngine::Core {
	std::string fontName;
	int fontSize;
	int highDPIScaleFactor;
	GLFWwindow* createWindow(RENDER_BACKEND backend) {
		using enum RENDER_BACKEND;
		GLFWmonitor* monitor = NULL;//glfwGetPrimaryMonitor(); // This allows for full screen, but it messes with launch time
		GLFWwindow* window = NULL;

		if (backend == DX12) {
			MirielEngine::Utils::GlobalLogger->log("Creating a DX12 Window.");
			// TODO: Set Hints on Window?
			 window = glfwCreateWindow(1920, 1080, "Miriel Engine - DirectX12", monitor, NULL);
		} else if (backend == OPENGL) {
			MirielEngine::Utils::GlobalLogger->log("Creating an OpenGL Window.");
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			window = glfwCreateWindow(1920, 1080, "Miriel Engine - OpenGL", monitor, NULL);
		} else if (backend == VULKAN) {
			MirielEngine::Utils::GlobalLogger->log("Creating a Vulkan Window.");
			// TODO: Set Hints on Window...
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			// TODO: More Hints
			window = glfwCreateWindow(1920, 1080, "Miriel Engine - Vulkan", monitor, NULL);
		} else {
			// This would be Metal once it is added
			MirielEngine::Utils::GlobalLogger->log("Creating a Metal Window.");
			// TODO: Set Hints on Window...
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			// TODO: More Hints
			window = glfwCreateWindow(1920, 1080, "Miriel Engine - Metal", monitor, NULL);
		}

		glfwSwapInterval(0);

		if (window == NULL) {
			throw MirielEngine::Errors::WindowCreationError("Failed to Create Window.");
		}

		return window;
	}

	void setWindowCallbacks(GLFWwindow* window) {
		// TODO: Set these with Custom Callbacks
		// TODO: Do Char Callback to Close Window, Then Sizing, Framebuffer, Refresh Callbacks, Others can Wait
		glfwSetKeyCallback(window, MirielEngine::Utils::WindowCallbacks::keyCallBack);
		// TODO: Three of these are probably going to have to be created by the backends
		glfwSetFramebufferSizeCallback(window, NULL);
		glfwSetWindowSizeCallback(window, NULL);
		glfwSetJoystickCallback(NULL);
		glfwSetWindowRefreshCallback(window, NULL);
		glfwSetMouseButtonCallback(window, MirielEngine::Utils::WindowCallbacks::mouseButtonCallBack);
		glfwSetScrollCallback(window, MirielEngine::Utils::WindowCallbacks::mouseScrollCallBack);
		glfwSetCursorPosCallback(window, MirielEngine::Utils::WindowCallbacks::mouseMovementCallBack);
		return;
	}
}
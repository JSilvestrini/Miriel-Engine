#include "Utils/MirielEngineWindow.hpp"
#include "Utils/WindowCallbacks.hpp"

namespace MirielEngine::Core {
	GLFWwindow* createWindow(RENDER_BACKEND backend) {
		using enum RENDER_BACKEND;
		GLFWmonitor* monitor = NULL;//glfwGetPrimaryMonitor();
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
		} else {
			MirielEngine::Utils::GlobalLogger->log("Creating a Vulkan Window.");
			// TODO: Set Hints on Window...
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			// TODO: More Hints
			window = glfwCreateWindow(1920, 1080, "Miriel Engine - Vulkan", monitor, NULL);
		}

		if (window == NULL) {
			throw MirielEngine::Errors::WindowCreationError("Failed to Create Window.");
		}

		return window;
	}

	void setWindowCallbacks(GLFWwindow* window) {
		// TODO: Set these with Custom Callbacks
		// TODO: Do Char Callback to Close Window, Then Sizing, Framebuffer, Refresh Callbacks, Others can Wait
		glfwSetKeyCallback(window, MirielEngine::Utils::WindowCallbacks::keyCallBack);
		glfwSetFramebufferSizeCallback(window, NULL);
		glfwSetWindowSizeCallback(window, NULL);
		glfwSetJoystickCallback(NULL);
		glfwSetWindowRefreshCallback(window, NULL);
		glfwSetMouseButtonCallback(window, NULL);
		glfwSetScrollCallback(window, NULL);
		glfwSetCursorPosCallback(window, NULL);
		return;
	}
}
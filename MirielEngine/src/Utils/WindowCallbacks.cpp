#include "Utils/WindowCallbacks.hpp"

namespace MirielEngine::Utils::WindowCallbacks {
	void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
	}

	// TODO: Finish these, might need to be implemented within the backends to have access to the camera
	void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods) {
		// Is it possible to get access to the current scene or the core of the application in this function?
		// Would need to get to one of them in order to get the camera within the scene, maybe I can have a
		// shared pointer called activeScene or something somewhere that is a pointer to the scene that was loaded

		// Maybe the callbacks can be placed within the scene and then passed as callables into another function?
	}

	void mouseEnterCallBack(GLFWwindow* window, int entered) {}

	void mouseScrollCallBack(GLFWwindow* window, double xOffset, double yOffset) {}

	void mouseMovementCallBack(GLFWwindow* window, double xPos, double yPos) {}
}
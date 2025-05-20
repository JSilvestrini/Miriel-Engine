#include "Utils/WindowCallbacks.hpp"

namespace MirielEngine::Utils::WindowCallbacks {
	void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
	}
}
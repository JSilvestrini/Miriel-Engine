#pragma once

#include "GLFW/glfw3.h"

namespace MirielEngine::Utils::WindowCallbacks {
	void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
	void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods);
	void mouseEnterCallBack(GLFWwindow* window, int entered);
	void mouseScrollCallBack(GLFWwindow* window, double xOffset, double yOffset);
	void mouseMovementCallBack(GLFWwindow* window, double xPos, double yPos);
}
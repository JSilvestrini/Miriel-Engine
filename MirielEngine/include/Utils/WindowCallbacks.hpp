#pragma once

#include "GLFW/glfw3.h"

namespace MirielEngine::Utils::WindowCallbacks {
	void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
}
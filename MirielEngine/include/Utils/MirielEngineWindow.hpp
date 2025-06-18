#pragma once
#include "Utils/MirielEngineLogger.hpp"

#include <GLFW/glfw3.h>

enum class RENDER_BACKEND {
	DX12,
	OPENGL,
	VULKAN,
	METAL
};

namespace MirielEngine::Core {
	GLFWwindow* createWindow(RENDER_BACKEND backend);
	void setWindowCallbacks(GLFWwindow* window);
}
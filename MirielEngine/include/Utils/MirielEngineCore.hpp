#pragma once
#include "CustomErrors/MirielEngineErrors.hpp"

#if _WIN64
#include "OpenGL/OpenGLApplication.hpp"
// can include vulkan and direct x here too?
#elif __APPLE__
#include "Metal/MetalApplication.hpp"
#endif

namespace MirielEngine::Core {
	void StartBackend(RENDER_BACKEND backend);
}
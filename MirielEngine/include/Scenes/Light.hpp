#pragma once

#include <glm/vec3.hpp>

namespace MirielEngine::Core {
	struct Light {
		glm::vec3 pos;
		glm::vec3 color;
	};
}
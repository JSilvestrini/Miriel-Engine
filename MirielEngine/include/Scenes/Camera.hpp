#pragma once

#include <glm/vec3.hpp>

namespace MirielEngine::Core {
	struct Camera {
		glm::vec3 pos;
		glm::vec3 target;
		glm::vec3 up;
		glm::vec3 camUp;
	};
}
#pragma once

#include <glm/vec3.hpp>

namespace MirielEngine::Core {
	struct Light {
		glm::vec3 color;
		glm::vec3 value;
		short type; // short == 0 means value is a position, short == 1 means that value is a direction
	};
}
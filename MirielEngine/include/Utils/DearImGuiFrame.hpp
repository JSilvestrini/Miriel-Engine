#pragma once

#include <functional>
#include <memory>

#include "DearImGui/imgui.h"

#include "Scenes/Objects.hpp"

namespace MirielEngine::Utils {
	using ImGuiImplementationFunction = std::function<void ()>;
	class GUI {
	private:
		ImGuiIO& io;
		std::string selectedName;
		std::weak_ptr<MirielEngine::Core::Scene> scene;
		/*
			Current List will keep track of what list I am currently looking at
				0: directional lights
				1: point lights
				2+: objects in scene starting at 0

			Current Object will let me know what item in the list I am looking at, so 0 is 0th index, etc.
		*/
		size_t currentList;
		size_t currentObject;
	public:
		GUI(std::shared_ptr<MirielEngine::Core::Scene> s);
		~GUI();
		void generateFrame(const ImGuiImplementationFunction& implFunction);
	};
};
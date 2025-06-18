#include "Utils/DearImGuiFrame.hpp"

#include <glm/gtc/type_ptr.hpp>
#include "DearImGui/imgui_impl_glfw.h"

#if _WIN64
#include "DearImGui/imgui_impl_opengl3.h"
#include "DearImGui/imgui_impl_vulkan.h"
#include "DearImGui/imgui_impl_dx12.h"
#elif __APPLE__
#include "DearImGui/imgui_impl_metal.h"
#endif

#include "Utils/MirielEngineLogger.hpp"

// Could create an initializer here with a scene pointer and init function to set up everything

namespace MirielEngine::Utils {
	GUI::GUI(std::shared_ptr<MirielEngine::Core::Scene> s) : scene(s), io(ImGui::GetIO()) {
		MirielEngine::Utils::GlobalLogger->log("Creating GUI Helper Class.");
		currentObject = 0;
		currentList = 0;
	}

	GUI::~GUI() {
		MirielEngine::Utils::GlobalLogger->log("Destroying GUI Helper Class.");
	}

	void GUI::generateFrame(const ImGuiImplementationFunction& implFunction) {
		implFunction();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			ImGui::BeginMainMenuBar();
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Scene")) {
					// new scene
					// TODO: need access to a core function?
					// have to use a function that clears everything from shaders to buffers
				}

				if (ImGui::MenuItem("Save Scene")) {
					auto sharedScene = scene.lock();
					sharedScene->saveScene();
				}

				if (ImGui::MenuItem("Save Scene As...")) {
					auto sharedScene = scene.lock();
					sharedScene->saveSceneAs();
				}

				if (ImGui::MenuItem("Load Scene")) {
					// load new scene
					// TODO: need access to a core function?
					// check new scene notes
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();

		}

		{
			// TODO: Save Scene File Function
			// TODO: Allow Loading of Scenes
			// TODO: Move and Lock to far left

			// TODO: Turn window into a class
			// TODO: Give window a weak pointer to the scene
			// TODO: Make callbacks use the weak pointer
			// TODO: Could supply function pointers to the window class
			// TODO: Window class could maintain ownership of glfw window
			// TODO: This would allow use of unique_ptr for memory management and RAII

			auto sharedScene = scene.lock();
			if (!sharedScene) { return; }

			ImGui::Begin("Scene Tree");

			if (ImGui::CollapsingHeader("Directional Lights")) {
				size_t numDLights = sharedScene->directionalLights.size();
				for (size_t i = 0; i < numDLights; i++) {
					std::ostringstream oss;
					oss << "Directional Light " << i;
					if (ImGui::Button(oss.str().c_str())) {
						selectedName = oss.str();
						currentList = 0;
						currentObject = i;
					}
				}
				if (ImGui::Button("Add Directional Light")) {
					sharedScene->addDirectionalLight();
				}
			}

			ImGui::Separator();

			if (ImGui::CollapsingHeader("Point Lights")) {
				size_t numPLights = sharedScene->pointLights.size();
				for (size_t i = 0; i < numPLights; i++) {
					std::ostringstream oss;
					oss << "Point Light " << i;
					if (ImGui::Button(oss.str().c_str())) {
						selectedName = oss.str();
						currentList = 1;
						currentObject = i;
					}
				}
				if (ImGui::Button("Add Point Light")) {
					sharedScene->addPointLight();
				}
			}

			ImGui::Separator();

			for (const auto& nameObject : sharedScene->loadedObjectNames) {
				std::string oName = sharedScene->objects[nameObject.second].getName();
				if (ImGui::CollapsingHeader(oName.c_str())) {
					for (size_t i = 0; i < sharedScene->objectInstances[nameObject.second].size(); i++) {
						std::ostringstream oss;
						oss << oName << " Instance " << i;
						if (ImGui::Button(oss.str().c_str())) {
							selectedName = oss.str();
							currentList = nameObject.second + 2;
							currentObject = i;
						}
					}
					std::ostringstream oss;
					oss << "Add " << oName;
					if (ImGui::Button(oss.str().c_str())) {
						// need a specific function to add new object for each backend to ensure that the shader/program/pipeline exists for it
						sharedScene->addObjectInstance(nameObject.second);
					}
				}
				ImGui::Separator();
			}
			if (ImGui::Button("Add New Object")) {
				// want to open windows menu, allow user to choose the object, then save the object path
				// Need to create custom load object function and add in a new instance without a file*
				sharedScene->addObject();
			}

			ImGui::End();
		}

		{
			// this would be the right pane, show the different attributes of the object
			// TODO: Move and lock to the right side
			ImGui::Begin("Selected Item");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

			auto sharedScene = scene.lock();
			if (!sharedScene) {
				ImGui::End();
				return;
			}

			if (selectedName.empty()) {
				ImGui::End();
				return;
			}

			if (ImGui::CollapsingHeader(selectedName.c_str())) {
				switch (currentList) {
					case 0:
						ImGui::ColorPicker3("Color", glm::value_ptr(sharedScene->directionalLights[currentObject].color));
						ImGui::InputFloat3("Direction", glm::value_ptr(sharedScene->directionalLights[currentObject].value), "%0.1f");
						break;
					case 1:
						ImGui::ColorPicker3("Color", glm::value_ptr(sharedScene->pointLights[currentObject].color));
						ImGui::InputFloat3("Position", glm::value_ptr(sharedScene->pointLights[currentObject].value), "%0.1f");
						break;
					default:
						ImGui::InputFloat3("Translation", glm::value_ptr(sharedScene->objectInstances[currentList - 2][currentObject].vTranslation), "%0.1f");
						sharedScene->objectInstances[currentList - 2][currentObject].updateTranslation();
						ImGui::InputFloat3("Scale", glm::value_ptr(sharedScene->objectInstances[currentList - 2][currentObject].vScale), "%0.1f");
						sharedScene->objectInstances[currentList - 2][currentObject].updateScale();
						ImGui::InputFloat3("Rotation", glm::value_ptr(sharedScene->objectInstances[currentList - 2][currentObject].vRotation), "%0.1f");
						sharedScene->objectInstances[currentList - 2][currentObject].updateRotation();

						ImGui::Text(sharedScene->objectInstances[currentList - 2][currentObject].vertexShaderName.c_str());
						if (ImGui::Button("Change Vertex Shader")) {
							sharedScene->switchVertShader(currentList - 2, currentObject);
						}

						ImGui::Text(sharedScene->objectInstances[currentList - 2][currentObject].fragmentShaderName.c_str());
						if (ImGui::Button("Change Fragment Shader")) {
							sharedScene->switchFragShader(currentList - 2, currentObject);
						}
						break;
				}
			}

			ImGui::End();
		}
	}
}
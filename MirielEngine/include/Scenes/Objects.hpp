#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

#include <assimp/types.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include "Camera.hpp"
#include "Light.hpp"

namespace MirielEngine::Core {
	using TextureLoadFunction = std::function<unsigned int(const std::string&)>;

	struct Texture {
		unsigned int ID;
		std::string type;	// TODO: replace with enum
		aiString path;		// TODO: Replace with something smaller
	};

	struct Vertex {
		glm::vec3 aPos;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 texCoord;
	};

	struct Object {
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;
		std::string vertexShaderID;
		std::string fragmentShaderID;
	};

	struct Particles {
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> colors;
		std::vector<float> lifetime;
		// give a certain program so that the particles choose their own shader, issue for Vulkan and D3D12 since they have entire pipelines
	};

	struct ObjectInstance {
		// materials or colors later?
		glm::mat4 translation;
		glm::mat4 scale;
		glm::quat rotation;
		unsigned int shaderID;
	};

	struct Scene {
		std::unordered_map<std::string, size_t> loadedObjectNames;
		std::unordered_map<size_t, std::vector<ObjectInstance>> objectInstances;
		std::vector<Object> objects;
		std::vector<Particles> particles;
		std::vector<Light> lights;
		Camera camera;

		void loadSceneFile(const std::string& sceneName, const TextureLoadFunction& textureLoader);
		void loadSceneObject(std::ifstream* sceneFile, const std::string& objName, const TextureLoadFunction& textureLoader);
		void loadSceneParticle(std::ifstream* sceneFile);
		void loadSceneCamera(std::ifstream* sceneFile);
		void loadSceneLight(std::ifstream* sceneFile);
	};
}
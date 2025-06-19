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
	using CleanGraphicsAPIFunction = std::function<void ()>;

	struct Texture {
		unsigned int ID;
		std::string type;	// TODO: replace with enum
		aiString path;		// TODO: Replace with something smaller?
	};

	struct Shader {
		size_t ID;
		size_t count; // reference count this so I can delete programs when there are 0 using it
		bool loaded;
	};

	struct Vertex {
		glm::vec3 aPos;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 texCoord;
	};

	struct Object {
		std::string vertexShaderName;
		std::string fragmentShaderName;
		std::string path;
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		std::string getName();
	};

	struct ParticleSpawner {
		std::vector<std::string> shaders;
		std::vector<glm::vec3> particlePositions;
		std::vector<float> particleLifetimes;
		glm::vec3 position;
		glm::vec3 color;
		// give a certain program so that the particles choose their own shader, issue for Vulkan and D3D12 since they have entire pipelines
	};

	struct ObjectInstance {
		// materials or colors later?
		// TODO: Check if shader name is default shader for object, ignore if true
		std::string vertexShaderName;
		std::string fragmentShaderName;
		Shader shaderProgram; // TODO: Check wording and feasibility
		glm::mat4 mTranslation;
		glm::mat4 mScale;
		glm::quat mRotation;
		glm::vec3 vTranslation;
		glm::vec3 vScale;
		glm::vec3 vRotation;

		ObjectInstance(const Object& o);
		~ObjectInstance();

		void updateTranslation();
		void updateScale();
		void updateRotation();
	};

	struct Scene {
		std::unordered_map<std::string, size_t> loadedObjectNames; // <- Could potentially be replaced by a vector assuming that objects are grouped properly in file
		std::unordered_map<size_t, std::vector<ObjectInstance>> objectInstances;
		std::unordered_map<std::string, Shader> loadedShaderCombinations;
		TextureLoadFunction textureLoader;
		CleanGraphicsAPIFunction clearAPIFunction;
		std::string scenePath;
		std::vector<Object> objects;
		std::vector<ParticleSpawner> particles;

		std::vector<Light> pointLights;
		std::vector<Light> directionalLights;

		Camera camera;

		void loadSceneFile(const std::string& sceneName);
		void loadSceneObject(std::ifstream* sceneFile, const std::string& objName);
		void loadSceneParticle(std::ifstream* sceneFile);
		void loadSceneCamera(std::ifstream* sceneFile);
		void loadSceneLight(std::ifstream* sceneFile);

		void addPointLight();
		void addDirectionalLight();
		void addObjectInstance(size_t index);
		void switchVertShader(size_t objectIndex, size_t instanceIndex);
		void switchFragShader(size_t objectIndex, size_t instanceIndex);
		void addObject();

		void saveScene();
		void saveSceneAs();
		void newScene();
		void loadScene();
	};
}
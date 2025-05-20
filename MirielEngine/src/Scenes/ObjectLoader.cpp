#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <stack>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Scenes/ObjectLoader.hpp"
#include "Utils/MirielEngineLogger.hpp"
#include "CustomErrors/MirielEngineErrors.hpp"

//TODO: include assimp here

namespace MirielEngine::Core {
	void loadObject(const std::string& objectName, MirielEngine::Core::Object* object, const TextureLoadFunction& textureLoader) {
		std::string location = std::filesystem::current_path().string() + "/src/Assets/Models/" + objectName;
		MirielEngine::Utils::GlobalLogger->log("Loading in Object: " + objectName);
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(location, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::ostringstream os;
			os << "Error Loading Object With Assimp: " << importer.GetErrorString() << ".";
			throw MirielEngine::Errors::ObjectLoaderError(os.str().c_str());
		}

		processNode(scene->mRootNode, scene, object, textureLoader);
	}

	void processNode(aiNode* node, const aiScene* scene, MirielEngine::Core::Object* object, const TextureLoadFunction& textureLoader) {
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			processMesh(scene->mMeshes[node->mMeshes[i]], scene, object, textureLoader);
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene, object, textureLoader);
		}
	}

	void processMesh(aiMesh* mesh, const aiScene* scene, MirielEngine::Core::Object* object, const TextureLoadFunction& textureLoader) {
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex v{};
			v.aPos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			v.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

			if (mesh->mTextureCoords[0]) {
				v.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			}

			v.color = glm::vec3(1.0f, 1.0f, 1.0f);
			object->vertices.push_back(v);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				object->indices.push_back(face.mIndices[j]);
			}
		}

		if (mesh->mMaterialIndex >= 0) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			loadMaterials(material, aiTextureType_DIFFUSE, "texture_diffuse", object, textureLoader);
			loadMaterials(material, aiTextureType_SPECULAR, "texture_specular", object, textureLoader);
		}
	}

	void loadMaterials(aiMaterial* material, aiTextureType type, std::string typeName,
						MirielEngine::Core::Object* object, const TextureLoadFunction& textureLoader) {

		for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
			// TODO: Changed textures from shared pointer to straight in memory, check once objects have textures
			aiString str;
			material->GetTexture(type, i, &str);
			Texture texture{};
			texture.ID = textureLoader(str.C_Str());
			texture.type = typeName;
			texture.path = str;
			object->textures.push_back(texture);
		}
	}

	// TODO: Compress Texture Function?
	void compressTexture(const std::string& textureName) { return; }

	void Scene::loadSceneObject(std::ifstream* sceneFile, const std::string& objName, const TextureLoadFunction& textureLoader) {
		std::stack<char> braces{};
		if (!loadedObjectNames.contains(objName)) {
			MirielEngine::Utils::GlobalLogger->log("Loading Object: " + objName);
			Object object{};
			MirielEngine::Core::loadObject(objName, &object, textureLoader);
			std::string temp;
			*sceneFile >> temp;
			braces.push(temp[0]);

			*sceneFile >> object.vertexShaderID;
			*sceneFile >> object.fragmentShaderID;

			this->objects.push_back(object);
			this->loadedObjectNames[objName] = this->objects.size() - 1;
		}

		size_t currentObject = this->loadedObjectNames[objName];

		while (!braces.empty()) {
			ObjectInstance instance{};

			// push identity
			instance.translation = glm::mat4(1.0f);
			instance.rotation = glm::mat4(1.0f);
			instance.scale = glm::mat4(1.0f);

			this->objectInstances[currentObject].push_back(instance);
			break;
		}

		// TODO: While stack not empty, read in transforms an input them as new instances
		/*
		else if (tag == "t") {
				std::string x, y, z;
				sceneFile >> x >> y >> z;
				glm::vec3 v(std::stof(x), std::stof(y), std::stof(z));
				this->objectInstances[currentObject].back().translation = glm::translate(this->objectInstances[currentObject].back().translation, v);
			} else if (tag == "r") {
				std::string x, y, z;
				sceneFile >> x >> y >> z;
				glm::vec3 v(glm::radians(std::stof(x)), glm::radians(std::stof(y)), glm::radians(std::stof(z)));
				this->objectInstances[currentObject].back().rotation = glm::quat(v);
			} else if (tag == "s") {
				std::string x, y, z;
				sceneFile >> x >> y >> z;
				glm::vec3 v(std::stof(x), std::stof(y), std::stof(z));
				this->objectInstances[currentObject].back().scale = glm::scale(this->objectInstances[currentObject].back().scale, v);
		*/
	}

	void Scene::loadSceneParticle(std::ifstream* sceneFile) {}

	void Scene::loadSceneCamera(std::ifstream* sceneFile) {
		std::string x, y, z;
		*sceneFile >> x >> y >> z;
		camera.pos = glm::vec3(std::stof(x), std::stof(y), std::stof(z));
		*sceneFile >> x >> y >> z;
		camera.target = glm::vec3(std::stof(x), std::stof(y), std::stof(z));
		camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 dir = glm::normalize(camera.pos - camera.target);
		glm::vec3 right = glm::normalize(glm::cross(camera.up, dir));
		camera.camUp = glm::cross(dir, right);
	}

	void Scene::loadSceneLight(std::ifstream* sceneFile) {}

	// TODO: Complete function:
	void Scene::loadSceneFile(const std::string& sceneName, const TextureLoadFunction& textureLoader) {
		MirielEngine::Utils::GlobalLogger->log("Opening Scene File " + sceneName  + ".");
		std::string name = std::filesystem::current_path().string() + "/src/Assets/Scenes/" + sceneName;
		std::ifstream sceneFile(name);

		if (!sceneFile.is_open() || sceneFile.bad()) {
			std::ostringstream os;
			os << "Failed to Load Scene File " << sceneName << ".";
			throw MirielEngine::Errors::ObjectLoaderError(os.str().c_str());
		}

		MirielEngine::Utils::GlobalLogger->log(sceneName + " Successfully Opened.");

		std::string objName;
		size_t currentObject;

		while (sceneFile.good() && !sceneFile.eof()) {
			std::string tag;
			sceneFile >> tag;

			if (tag.ends_with(".obj") || tag.ends_with(".gltf") || tag.ends_with(".glb")) {
				loadSceneObject(&sceneFile, tag, textureLoader);
			} else if (tag == "c") {
				loadSceneCamera(&sceneFile);
			} else if (tag == "-l") {
				// TODO: Add in lights to the scene
				continue;
			} else if (tag == "p") {
				// particle
				continue;
			}
		}

		// TODO: set textures next

		MirielEngine::Utils::GlobalLogger->log(sceneName + " Successfully Loaded.");

		sceneFile.close();
	}
}
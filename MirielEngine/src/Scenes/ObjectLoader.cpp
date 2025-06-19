#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <stack>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

// https://github.com/btzy/nativefiledialog-extended
#include <nfd.h>

#include "Scenes/ObjectLoader.hpp"
#include "Utils/MirielEngineLogger.hpp"
#include "CustomErrors/MirielEngineErrors.hpp"

/*
Note:	In order to make a 2D graphics engine, I would want to use orthographic projection rather than normal perspective.
		This would make it so that the 2D shapes that hold the sprites would properly line up on the screen,

		Ex:	A large rectangle that covers the 'camera' space would be the background, and I could increase
			the Z coordinate to make the other sprites render in front of it, so the player could be at Z = 0,
			which would render the player on top if the background. All it needs are sprites with transparent pixels to make
			parts of the sprite invisible
*/

namespace MirielEngine::Core {
	void loadObject(const std::string& objectName, MirielEngine::Core::Object* object, const TextureLoadFunction& textureLoader) {
		std::string location = objectName;
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

	void Scene::loadSceneObject(std::ifstream* sceneFile, const std::string& objName) {
		std::stack<char> braces{};
		if (!loadedObjectNames.contains(objName)) {
			Object object{};
			MirielEngine::Core::loadObject(objName, &object, textureLoader);
			std::string tag;
			*sceneFile >> tag;
			braces.push(tag[0]);
			object.path = objName;

			this->objects.push_back(object);
			this->loadedObjectNames[objName] = this->objects.size() - 1;
		}

		size_t currentObject = this->loadedObjectNames[objName];

		std::string defaultVertShader;
		std::string defaultFragShader;

		*sceneFile >> defaultVertShader;
		*sceneFile >> defaultFragShader;

		this->objects[currentObject].fragmentShaderName = defaultFragShader;
		this->objects[currentObject].vertexShaderName = defaultVertShader;

		std::string shaderPair = defaultVertShader + " " + defaultFragShader;
		if (!loadedShaderCombinations.contains(shaderPair)) {
			loadedShaderCombinations[shaderPair] = Shader{ 0, false };
		}

		while (true) {
			std::string tag;
			*sceneFile >> tag;

			if (tag[0] == '{') {
				braces.push('{');
			} else {
				braces.pop();
			}

			if (braces.empty()) {
				break;
			}

			ObjectInstance instance{objects[currentObject]};

			// push identity
			instance.fragmentShaderName = defaultFragShader;
			instance.vertexShaderName = defaultVertShader;

			while (true) {
				*sceneFile >> tag;

				if (tag[0] == 't') {
					std::string x, y, z;
					*sceneFile >> x >> y >> z;
					glm::vec3 v(std::stof(x), std::stof(y), std::stof(z));
					instance.vTranslation = v;
					instance.mTranslation = glm::translate(instance.mTranslation, v);
				} else if (tag[0] == 'r') {
					std::string x, y, z;
					*sceneFile >> x >> y >> z;
					glm::vec3 v(std::stof(x), std::stof(y), std::stof(z));
					instance.vRotation = v;
					instance.mRotation = glm::quat(glm::radians(v));
				} else if (tag[0] == 's') {
					std::string x, y, z;
					*sceneFile >> x >> y >> z;
					glm::vec3 v(std::stof(x), std::stof(y), std::stof(z));
					instance.vScale = v;
					instance.mScale = glm::scale(instance.mScale, instance.vScale);
				} else if (tag[0] == '}') {
					braces.pop();
					break;
				} else if (tag[0] == 'v') {
					*sceneFile >> instance.vertexShaderName;
				} else if (tag[0] == 'f') {
					*sceneFile >> instance.fragmentShaderName;

				}
			}

			shaderPair = instance.vertexShaderName + " " + instance.fragmentShaderName;
			if (!loadedShaderCombinations.contains(shaderPair)) {
				loadedShaderCombinations[shaderPair] = Shader{ 0, false };
			}

			this->objectInstances[currentObject].push_back(instance);
		}

	}

	void Scene::loadSceneParticle(std::ifstream* sceneFile) {
		MirielEngine::Utils::GlobalLogger->log("Loading in Particle Spawners.");
		std::stack<char> braces{};
		std::string tag;
		*sceneFile >> tag;

		braces.push(tag[0]);

		while (true) {
			*sceneFile >> tag;

			if (tag[0] == '{') {
				braces.push(tag[0]);
			} else if (tag[0] == '}') {
				break;
			}

			ParticleSpawner newParticleSpawner{};

			*sceneFile >> tag;
			newParticleSpawner.shaders.push_back(tag);

			*sceneFile >> tag;
			newParticleSpawner.shaders.push_back(tag);

			newParticleSpawner.position = glm::vec3(0.0, 0.0, 0.0);
			newParticleSpawner.color = glm::vec3(1.0, 1.0, 1.0);

			while (true) {
				*sceneFile >> tag;

				if (tag[0] == 'p') {
					std::string x, y, z;
					*sceneFile >> x >> y >> z;
					glm::vec3 v(std::stof(x), std::stof(y), std::stof(z));
					newParticleSpawner.position = v;
				} else if (tag[0] == 'c') {
					std::string x, y, z;
					*sceneFile >> x >> y >> z;
					glm::vec3 v(std::stof(x), std::stof(y), std::stof(z));
					newParticleSpawner.color = v;
				} else if (tag[0] == '}') {
					braces.pop();
					break;
				}
			}

			particles.push_back(newParticleSpawner);
		}
	}

	void Scene::loadSceneCamera(std::ifstream* sceneFile) {
		MirielEngine::Utils::GlobalLogger->log("Loading in Camera.");
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

	void Scene::loadSceneLight(std::ifstream* sceneFile) {
		MirielEngine::Utils::GlobalLogger->log("Loading in Lights.");
		std::stack<char> braces{};
		std::string tag;
		*sceneFile >> tag;

		braces.push(tag[0]);

		while (true) {
			Light newLight{};

			*sceneFile >> tag;

			if (tag[0] == '{') {
				braces.push(tag[0]);
			} else if (tag[0] == '}') {
				break;
			}

			while (true) {
				*sceneFile >> tag;

				if (tag[0] == 'p') {
					newLight.type = 0;
				} else if (tag[0] == 'd') {
					newLight.type = 1;
				}

				if (tag[0] == 'c') {
					std::string x, y, z;
					*sceneFile >> x >> y >> z;
					glm::vec3 v(std::stof(x), std::stof(y), std::stof(z));
					newLight.color = v;
				} else if (tag[0] == 'd' || tag[0] == 'p') {
					std::string x, y, z;
					*sceneFile >> x >> y >> z;
					glm::vec3 v(std::stof(x), std::stof(y), std::stof(z));
					newLight.value = v;
				} else if (tag[0] == '}') {
					braces.pop();
					break;
				}
			}

			if (newLight.type == 1) {
				directionalLights.push_back(newLight);
			} else {
				pointLights.push_back(newLight);
			}
		}
	}

	void Scene::loadSceneFile(const std::string& sceneName) {
		// TODO: If there is a scene already loaded, need to reset the graphics API stuff, like VBO's, programs/ pipelines, etc.
		MirielEngine::Utils::GlobalLogger->log("Opening Scene File " + sceneName  + ".");
		std::string name = sceneName;
		scenePath = name;
		std::ifstream sceneFile(name);

		if (!sceneFile.is_open() || sceneFile.bad()) {
			std::ostringstream os;
			os << "Failed to Load Scene File " << sceneName << ".";
			throw MirielEngine::Errors::ObjectLoaderError(os.str().c_str());
		}

		MirielEngine::Utils::GlobalLogger->log(sceneName + " Successfully Opened.");

		std::string objName;

		while (sceneFile.good() && !sceneFile.eof()) {
			std::string tag;
			sceneFile >> tag;

			if (tag.ends_with(".obj") || tag.ends_with(".gltf") || tag.ends_with(".glb")) {
				loadSceneObject(&sceneFile, tag);
			} else if (tag == "c") {
				loadSceneCamera(&sceneFile);
			} else if (tag == "l") {
				loadSceneLight(&sceneFile);
			} else if (tag == "p") {
				loadSceneParticle(&sceneFile);
			}
		}

		// TODO: set textures next?

		MirielEngine::Utils::GlobalLogger->log(sceneName + " Successfully Loaded.");

		sceneFile.close();
	}

	ObjectInstance::ObjectInstance(const Object& o) {
		mTranslation = glm::mat4(1.0f);
		mRotation = glm::mat4(1.0f);
		mScale = glm::mat4(1.0f);
		vScale = glm::vec3(1.0f);
		vRotation = glm::vec3(0.0f);
		vTranslation = glm::vec3(0.0f);
		vertexShaderName = o.vertexShaderName;
		fragmentShaderName = o.fragmentShaderName;
	}

	ObjectInstance::~ObjectInstance() = default;

	void ObjectInstance::updateRotation() {
		mRotation = glm::quat(glm::radians(vRotation));
	}

	void ObjectInstance::updateScale() {
		mScale = glm::scale(glm::mat4(1), vScale);
	}

	void ObjectInstance::updateTranslation() {
		mTranslation = glm::translate(glm::mat4(1), vTranslation);
	}

	void Scene::addPointLight() {
		pointLights.push_back( Light{glm::vec3(0), glm::vec3(0), 0 });
	}

	void Scene::addDirectionalLight() {
		directionalLights.push_back(Light{ glm::vec3(0), glm::vec3(0), 1 });
	}

	void Scene::addObjectInstance(size_t index) {
		ObjectInstance i{objects[index]};
		objectInstances[index].push_back(i);

		if (i.vertexShaderName.empty() || i.fragmentShaderName.empty()) { return; }
		i.shaderProgram = loadedShaderCombinations[i.vertexShaderName + " " + i.fragmentShaderName];
	}

	void Scene::switchVertShader(size_t objectIndex, size_t instanceIndex) {
		MirielEngine::Utils::GlobalLogger->log("User is Adding New Vertex Shader.");
		nfdu8char_t* outPath;
		nfdu8filteritem_t filters[1] = { {"Vertex Shader File", "vert"} };
		nfdopendialogu8args_t args = { 0 };
		args.filterList = filters;
		args.filterCount = 1;
		MirielEngine::Utils::GlobalLogger->log("Opening Dialogue Box with NFD.");
		nfdresult_t res = NFD_OpenDialogU8_With(&outPath, &args);

		if (res != NFD_OKAY) {
			return;
		}

		std::ostringstream oss;
		oss << "User Selected New Item: " << outPath;
		MirielEngine::Utils::GlobalLogger->log(oss.str());

		objectInstances[objectIndex][instanceIndex].vertexShaderName = outPath;

		MirielEngine::Utils::GlobalLogger->log("New Vertex Shader Added.");

		if (objectInstances[objectIndex][instanceIndex].fragmentShaderName.empty()) { return; }

		std::string key = objectInstances[objectIndex][instanceIndex].vertexShaderName + " " + objectInstances[objectIndex][instanceIndex].fragmentShaderName;

		if (!loadedShaderCombinations.contains(key)) {
			loadedShaderCombinations[key] = Shader{0, false};
		}

		NFD_FreePathU8(outPath);
	}

	void Scene::switchFragShader(size_t objectIndex, size_t instanceIndex) {
		MirielEngine::Utils::GlobalLogger->log("User is Adding New Fragment Shader.");
		nfdu8char_t* outPath;
		nfdu8filteritem_t filters[1] = { {"Fragment Shader File", "frag"} };
		nfdopendialogu8args_t args = { 0 };
		args.filterList = filters;
		args.filterCount = 1;
		MirielEngine::Utils::GlobalLogger->log("Opening Dialogue Box with NFD.");
		nfdresult_t res = NFD_OpenDialogU8_With(&outPath, &args);

		if (res != NFD_OKAY) {
			return;
		}

		std::ostringstream oss;
		oss << "User Selected New Item: " << outPath;
		MirielEngine::Utils::GlobalLogger->log(oss.str());

		objectInstances[objectIndex][instanceIndex].fragmentShaderName = outPath;

		MirielEngine::Utils::GlobalLogger->log("New Fragment Shader Added.");

		if (objectInstances[objectIndex][instanceIndex].vertexShaderName.empty()) { return; }

		std::string key = objectInstances[objectIndex][instanceIndex].vertexShaderName + " " + objectInstances[objectIndex][instanceIndex].fragmentShaderName;

		if (!loadedShaderCombinations.contains(key)) {
			loadedShaderCombinations[key] = Shader{ 0, false };
		}

		NFD_FreePathU8(outPath);
	}

	void Scene::addObject() {
		MirielEngine::Utils::GlobalLogger->log("User is Adding New Object.");
		nfdu8char_t* outPath;
		nfdu8filteritem_t filters[1] = {{"Object Files", "obj,glb,gltf"}};
		nfdopendialogu8args_t args = {0};
		args.filterList = filters;
		args.filterCount = 1;
		MirielEngine::Utils::GlobalLogger->log("Opening Dialogue Box with NFD.");
		nfdresult_t res = NFD_OpenDialogU8_With(&outPath, &args);

		if (res != NFD_OKAY) {
			return;
		}

		std::ostringstream oss;
		oss << "User Selected New Item: " << outPath;
		MirielEngine::Utils::GlobalLogger->log(oss.str());

		if (loadedObjectNames.contains(outPath)) {
			return;
		}

		Object o{};
		o.path = outPath;

		if (!loadedShaderCombinations.empty()) {
			std::string loadedShader = loadedShaderCombinations.begin()->first;
			size_t splitIndex = loadedShader.find(' ');
			o.vertexShaderName = loadedShader.substr(0, splitIndex);
			o.fragmentShaderName = loadedShader.substr(splitIndex + 1, loadedShader.size() - o.vertexShaderName.size() - 1);
		}

		MirielEngine::Core::loadObject(outPath, &o, textureLoader);
		loadedObjectNames[outPath] = objects.size();
		objects.push_back(o);

		objectInstances[objects.size() - 1] = std::vector<ObjectInstance>{};
		addObjectInstance(objects.size() - 1);
		MirielEngine::Utils::GlobalLogger->log("New Object Has Been Added.");

		NFD_FreePathU8(outPath);
	}

	std::string Object::getName() {
		size_t i = path.rfind('/');
		if (i >= path.size()) {
			i = path.rfind("\\");
		}
		return path.substr(i + 1);
	}

	void Scene::saveScene() {
		if (scenePath.empty()) {
			nfdu8char_t* outPath;
			nfdu8filteritem_t filters[1] = { {"Miriel Engine Scene File", "mscn"} };
			nfdsavedialogu8args_t args = { 0 };
			args.filterList = filters;
			args.filterCount = 1;
			MirielEngine::Utils::GlobalLogger->log("Opening Dialogue Box with NFD.");
			nfdresult_t res = NFD_SaveDialogU8_With(&outPath, &args);

			if (res != NFD_OKAY) {
				return;
			}

			std::ostringstream oss;
			oss << "User Has Chosen a New File Name: " << outPath;
			MirielEngine::Utils::GlobalLogger->log(oss.str());

			scenePath = outPath;

			NFD_FreePathU8(outPath);
		}

		std::ostringstream oss;
		oss << "Saving Scene: " << scenePath;
		MirielEngine::Utils::GlobalLogger->log(oss.str());

		std::ofstream sceneFile(scenePath, std::ofstream::trunc | std::ofstream::out);

		if (!loadedShaderCombinations.empty()) {

			for (size_t i = 0; i < objects.size(); i++) {
				sceneFile << objects[i].path << "\n{\n";

				if (objects[i].vertexShaderName.empty() || objects[i].fragmentShaderName.empty()) {
					std::string loadedShader = loadedShaderCombinations.begin()->first;
					size_t splitIndex = loadedShader.find(' ');
					objects[i].vertexShaderName = loadedShader.substr(0, splitIndex);
					objects[i].fragmentShaderName = loadedShader.substr(splitIndex + 1, loadedShader.size() - objects[i].vertexShaderName.size() - 1);
				}

				sceneFile << "\t" << objects[i].vertexShaderName << " " << objects[i].fragmentShaderName;

				for (size_t j = 0; j < objectInstances[i].size(); j++) {
					sceneFile << "\n\t{";
					glm::vec3 t = objectInstances[i][j].vTranslation;
					glm::vec3 r = objectInstances[i][j].vRotation;
					glm::vec3 s = objectInstances[i][j].vScale;

					std::string ex = "\n\t\tf ";
					if (objectInstances[i][j].vertexShaderName != objects[i].vertexShaderName) {
						sceneFile << "\n\t\tv " << objectInstances[i][j].vertexShaderName;
						ex = " f ";
					}

					if (objectInstances[i][j].fragmentShaderName != objects[i].fragmentShaderName) {
						sceneFile << ex << objectInstances[i][j].fragmentShaderName;
					}

					ex = "\t";

					if (t != glm::vec3(0.0, 0.0, 0.0)) {
						sceneFile << "\n\t\tt " << t.x << " " << t.y << " " << t.z;
						ex = "\n\t";
					}

					if (r != glm::vec3(0.0, 0.0, 0.0)) {
						sceneFile << "\n\t\tr " << r.x << " " << r.y << " " << r.z;
						ex = "\n\t";
					}

					if (s != glm::vec3(1.0, 1.0, 1.0)) {
						sceneFile << "\n\t\ts " << s.x << " " << s.y << " " << s.z;
						ex = "\n\t";
					}

					sceneFile << ex << "}\n";
				}
				sceneFile << "}\n";
			}
		}

		sceneFile << "\nl {";
		std::string ex = "\t";
		for (size_t i = 0; i < directionalLights.size(); i++) {
			sceneFile << "\n\t{";
			sceneFile << "\n\t\tc " << directionalLights[i].color.x << " " << directionalLights[i].color.y << " " << directionalLights[i].color.z;
			sceneFile << "\n\t\td " << directionalLights[i].value.x << " " << directionalLights[i].value.y << " " << directionalLights[i].value.z << "\n";
			sceneFile << "\t}";
			ex = "\n";
		}

		for (size_t i = 0; i < pointLights.size(); i++) {
			sceneFile << "\n\t{";
			sceneFile << "\n\t\tc " << pointLights[i].color.x << " " << pointLights[i].color.y << " " << pointLights[i].color.z;
			sceneFile << "\n\t\tp " << pointLights[i].value.x << " " << pointLights[i].value.y << " " << pointLights[i].value.z << "\n";
			sceneFile << "\t}";
			ex = "\n";
		}
		sceneFile << ex << "}\n";

		sceneFile << "\np {";
		ex = "\t";
		for (size_t i = 0; i < particles.size(); i++) {
			sceneFile << "\n\t{\n\t\t";
			sceneFile << particles[i].shaders[0] << " " << particles[i].shaders[1] << "\n";
			sceneFile << "\t\tp " << particles[i].position.x << " " << particles[i].position.y << " " << particles[i].position.z << "\n";
			sceneFile << "\t\tc " << particles[i].color.x << " " << particles[i].color.y << " " << particles[i].color.z << "\n";
			sceneFile << "\t}";
			ex = "\n";
		}
		sceneFile << ex << "}\n";

		sceneFile << "\nc " << camera.pos.x << " " << camera.pos.y << " " << camera.pos.z << " ";
		sceneFile << camera.target.x << " " << camera.target.y << " " << camera.target.z;
		sceneFile.close();
	}

	void Scene::saveSceneAs() {
		MirielEngine::Utils::GlobalLogger->log("Saving Scene As...");
		nfdu8char_t* outPath;
		nfdu8filteritem_t filters[1] = { {"Miriel Engine Scene File", "mscn"} };
		nfdsavedialogu8args_t args = { 0 };
		args.filterList = filters;
		args.filterCount = 1;
		MirielEngine::Utils::GlobalLogger->log("Opening Dialogue Box with NFD.");
		nfdresult_t res = NFD_SaveDialogU8_With(&outPath, &args);

		if (res != NFD_OKAY) {
			return;
		}

		std::ostringstream oss;
		oss << "User Has Chosen a New File Name: " << outPath;
		MirielEngine::Utils::GlobalLogger->log(oss.str());

		scenePath = outPath;
		saveScene();

		NFD_FreePathU8(outPath);
	}

	void Scene::newScene() {
		// TODO: Needs to reset all buffers and unload everthing that needs to be unloaded.
		// Can use the open dialogue like in open loader, then call a reset function, then load scene and a build function from parent
		loadedObjectNames.clear();
		objectInstances.clear();
		objects.clear();
		loadedShaderCombinations.clear();
		pointLights.clear();
		directionalLights.clear();
		particles.clear();
		scenePath = "";
		clearAPIFunction();
	}

	void Scene::loadScene() {
		MirielEngine::Utils::GlobalLogger->log("User is Loading New Scene.");
		nfdu8char_t* outPath;
		nfdu8filteritem_t filters[1] = { {"Miriel Engine Scene File", "mscn"} };
		nfdopendialogu8args_t args = { 0 };
		args.filterList = filters;
		args.filterCount = 1;
		MirielEngine::Utils::GlobalLogger->log("Opening Dialogue Box with NFD.");
		nfdresult_t res = NFD_OpenDialogU8_With(&outPath, &args);

		if (res != NFD_OKAY) {
			return;
		}

		std::ostringstream oss;
		oss << "User Selected New Item: " << outPath;
		MirielEngine::Utils::GlobalLogger->log(oss.str());

		newScene();
		loadSceneFile(outPath);

		NFD_FreePathU8(outPath);
	}
}
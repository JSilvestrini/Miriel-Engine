#include <iostream>
#include <filesystem>

#include <stb_image.h>
#include <glm/gtc/type_ptr.hpp>

#include "OpenGL/Engine/Core/OpenGLCore.hpp"
#include "Scenes/ObjectLoader.hpp"
#include "CustomErrors/MirielEngineErrors.hpp"
#include "Utils/MirielEngineLogger.hpp"
#include "OpenGL/Engine/Utils/OpenGLUtils.hpp"

namespace MirielEngine::OpenGL {
	OpenGLCore::OpenGLCore() {
		// load in objects here
		// load in buffers
		MirielEngine::Utils::GlobalLogger->log("Creating OpenGL Core.");
		currentProgram = 0;
		scene = std::make_shared<MirielEngine::Core::Scene>();
		scene->textureLoader = ([this](const std::string& s) {return loadTexture(s); });
		scene->clearAPIFunction = ([this]() { return cleanUp(); });
		try {
			scene->newScene();
		} catch (MirielEngine::Errors::ObjectLoaderError& e) {
			MirielEngine::Utils::GlobalLogger->log(e.what());
		}

		createBuffers();
		createProgram();

		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK); // GL_FRONT
		//glFrontFace(GL_CW); // GL_CCW
	}

	void OpenGLCore::cleanUp() {
		for (GLuint program : programs) {
			glDeleteProgram(program);
		}

		glDeleteBuffers(objectVBOs.size(), objectVBOs.data());
		glDeleteBuffers(objectEBOs.size(), objectEBOs.data());
		glDeleteVertexArrays(objectVAOs.size(), objectVAOs.data());

		for (auto object : scene->objects) {
			for (auto texture : object.textures) {
				glDeleteTextures(1, &texture.ID);
			}
		}

		objectEBOs.clear();
		objectVAOs.clear();
		objectVBOs.clear();
		UBOIDs.clear();
		programs.clear();
	}

	OpenGLCore::~OpenGLCore() {
		MirielEngine::Utils::GlobalLogger->log("Destroying OpenGL Core.");
		cleanUp();
		glDeleteBuffers(UBOs.size(), UBOs.data());
		UBOs.clear();
	}

	void OpenGLCore::draw(int width, int height) {
		updateBuffers();
		updateProgram();

		if (programs.empty() || objectVAOs.empty() || objectEBOs.empty() || objectVBOs.empty() || UBOs.empty()) { return; }

		glm::mat4 view = glm::lookAt(scene->camera.pos, scene->camera.target, scene->camera.camUp);
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 1000.0f);

		glBindBuffer(GL_UNIFORM_BUFFER, UBOs[0]);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		for (const auto& objInstance : scene->objectInstances) {
			glBindVertexArray(objectVAOs[objInstance.first]);
			//glBindBuffer(GL_ARRAY_BUFFER, objectVBOs[objInstance.first]);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objectEBOs[objInstance.first]);

			// TODO: Make uniforms
			// TODO: Custom Shader Class, quickly add uniforms and stuff
			// TODO: Camera and Light Structs
			// TODO: Camera and Light in Shaders
			// TODO: Use some Textured objects
			// TODO: Check out UBO's to send data to shaders? Lights and the Unchanging view projections
			// TODO: Need to add in shadow pass for objects :(

			for (const auto& instance : objInstance.second) {
				glUseProgram(instance.shaderProgram.ID);
				glm::mat4 model = instance.mTranslation * glm::mat4_cast(instance.mRotation) * instance.mScale;
				int modelLoc = glGetUniformLocation(programs[currentProgram], "model");
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
				glDrawElements(GL_TRIANGLES, scene->objects[objInstance.first].indices.size(), GL_UNSIGNED_INT, 0);
			}
			glBindVertexArray(0);
		}
	}

	void OpenGLCore::updateBuffers() {
		// TODO: Check object loading again
		if (scene->objects.size() == objectVBOs.size()) { return; }

		size_t offset = scene->objects.size() - objectVBOs.size();

		for (size_t i = scene->objects.size() - offset; i < scene->objects.size(); i++) {
			unsigned int VBO;
			glGenBuffers(1, &VBO);
			objectVBOs.push_back(VBO);

			unsigned int EBO;
			glGenBuffers(1, &EBO);
			objectEBOs.push_back(EBO);

			unsigned int VAO;
			glGenVertexArrays(1, &VAO);
			objectVAOs.push_back(VAO);

			glBindVertexArray(objectVAOs[i]);

			glBindBuffer(GL_ARRAY_BUFFER, objectVBOs[i]);
			glBufferData(GL_ARRAY_BUFFER, scene->objects[i].vertices.size() * sizeof(MirielEngine::Core::Vertex), scene->objects[i].vertices.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objectEBOs[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, scene->objects[i].indices.size() * sizeof(unsigned int), scene->objects[i].indices.data(), GL_STATIC_DRAW);

			// location in shader, size of vertex, type, if normalized, space between vertex objects, offset of position data in vertex
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MirielEngine::Core::Vertex), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MirielEngine::Core::Vertex), (void*)(offsetof(MirielEngine::Core::Vertex, normal)));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MirielEngine::Core::Vertex), (void*)(offsetof(MirielEngine::Core::Vertex, color)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(MirielEngine::Core::Vertex), (void*)(offsetof(MirielEngine::Core::Vertex, texCoord)));
			glEnableVertexAttribArray(3);

			glBindVertexArray(0);
		}
	}

	void OpenGLCore::updateProgram() {
		size_t offset = scene->loadedShaderCombinations.size() - programs.size();

		if (offset == 0) { return; }

		for (auto& shaderCombination : scene->loadedShaderCombinations) {
			if (shaderCombination.second.loaded) { continue; }
			MirielEngine::Utils::GlobalLogger->log("Loading in a new Shader Combination.");
			size_t splitIndex = shaderCombination.first.find(' '); // need to find space index, split into 2 strings

			std::string vertShaderName;
			std::string fragShaderName;

			vertShaderName = shaderCombination.first.substr(0, splitIndex);
			fragShaderName = shaderCombination.first.substr(splitIndex + 1, shaderCombination.first.size() - vertShaderName.size() - 1);

			try {
				programs.push_back(MirielEngine::OpenGL::createShaderProgram(vertShaderName.c_str(), fragShaderName.c_str()));
				shaderCombination.second = MirielEngine::Core::Shader{ programs.back(), true};
			} catch (const MirielEngine::Errors::OpenGLUtilError& e) {
				throw MirielEngine::Errors::OpenGLError(e.what());
			}
		}

		for (auto& object : scene->objectInstances) {
			for (auto& objectInstance : object.second) {
				std::string key = objectInstance.vertexShaderName + " " + objectInstance.fragmentShaderName;
				objectInstance.shaderProgram = scene->loadedShaderCombinations[key];
			}
		}

		for (size_t i = scene->loadedShaderCombinations.size() - offset; i < scene->loadedShaderCombinations.size(); i++) {
			UBOIDs.push_back(glGetUniformBlockIndex(programs[i], "Matrices"));
			glUniformBlockBinding(programs[i], UBOIDs[i], 0);
		}
	}

	void OpenGLCore::createBuffers() {
		objectVBOs.resize(scene->objects.size());
		glGenBuffers(scene->objects.size(), objectVBOs.data());

		objectEBOs.resize(scene->objects.size());
		glGenBuffers(scene->objects.size(), objectEBOs.data());

		objectVAOs.resize(scene->objects.size());
		glGenVertexArrays(scene->objects.size(), objectVAOs.data());

		for (const auto& objInstance : scene->objectInstances) {
			glBindVertexArray(objectVAOs[objInstance.first]);
			// contains ObjectInstances, can get transforms from indices it
			glBindBuffer(GL_ARRAY_BUFFER, objectVBOs[objInstance.first]);
			glBufferData(GL_ARRAY_BUFFER, scene->objects[objInstance.first].vertices.size() * sizeof(MirielEngine::Core::Vertex), scene->objects[objInstance.first].vertices.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objectEBOs[objInstance.first]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, scene->objects[objInstance.first].indices.size() * sizeof(unsigned int), scene->objects[objInstance.first].indices.data(), GL_STATIC_DRAW);

			// location in shader, size of vertex, type, if normalized, space between vertex objects, offset of position data in vertex
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MirielEngine::Core::Vertex), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MirielEngine::Core::Vertex), (void*)(offsetof(MirielEngine::Core::Vertex, normal)));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MirielEngine::Core::Vertex), (void*)(offsetof(MirielEngine::Core::Vertex, color)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(MirielEngine::Core::Vertex), (void*)(offsetof(MirielEngine::Core::Vertex, texCoord)));
			glEnableVertexAttribArray(3);

			glBindVertexArray(0);
		}

		// Create UBOs for each program
		for (int i = 0; i < programs.size(); i++) {
			UBOIDs.push_back(glGetUniformBlockIndex(programs[i], "Matrices"));
			glUniformBlockBinding(programs[i], UBOIDs[i], 0);
		}

		unsigned int matrixUBO;
		glGenBuffers(1, &matrixUBO);

		glBindBuffer(GL_UNIFORM_BUFFER, matrixUBO);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 0, matrixUBO, 0, 2 * sizeof(glm::mat4));
		UBOs.push_back(matrixUBO);
	}

	void OpenGLCore::createProgram() {
		updateProgram();
	}

	void OpenGLCore::nextProgram() {
		currentProgram++;
		if (currentProgram >= programs.size()) { currentProgram = 0; }
	}

	void OpenGLCore::previousProgram() {
		if (currentProgram == 0) { currentProgram = programs.size(); }
		currentProgram--;
	}

	unsigned int OpenGLCore::loadTexture(const std::string& textureName) {
		std::cout << "Load Texture Called!" << std::endl;
		std::string location = std::filesystem::current_path().string() + "/src/Assets/Models/" + textureName;

		unsigned int texID;
		glGenTextures(1, &texID);

		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(textureName.c_str(), &texWidth, &texHeight, &texChannels, 0);

		if (!pixels) {
			stbi_image_free(pixels);
			std::ostringstream os;
			os << "Failed to Load Texture Located at: " << location << ".";
			MirielEngine::Utils::GlobalLogger->log(os.str());
			throw MirielEngine::Errors::OpenGLError(os.str().c_str());
		}

		GLenum format;
		if (texChannels == 1) {
			format = GL_RED;
		} else if (texChannels == 3) {
			format = GL_RGB;
		} else if (texChannels == 4) {
			format = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, texID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format, GL_UNSIGNED_BYTE, pixels);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(pixels);

		return texID;
	}

	std::shared_ptr<MirielEngine::Core::Scene> OpenGLCore::getScene() {
		return scene;
	}
}
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
		currentProgram = 0;
		scene = std::make_unique<MirielEngine::Core::Scene>();
		try {
			scene->loadSceneFile("newTest.txt", ([this](const std::string& s) {return loadTexture(s);}));
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

	OpenGLCore::~OpenGLCore() {
		// destroy buffers
		// TODO:
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
	}

	void OpenGLCore::draw(int width, int height) {
		glUseProgram(programs[currentProgram]);

		glm::mat4 view = glm::lookAt(scene->camera.pos, scene->camera.target, scene->camera.camUp);
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);

		for (const auto& objInstance : scene->objectInstances) {
			glBindVertexArray(objectVAOs[objInstance.first]);
			//glBindBuffer(GL_ARRAY_BUFFER, objectVBOs[objInstance.first]);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objectEBOs[objInstance.first]);

			// TODO: Add in Normals into vertexArray
			// TODO: Make uniforms
			// TODO: Custom Shader Class, quickly add uniforms and stuff
			// TODO: Work on Adding Camera & Lights to Scene
			// TODO: Camera and Light Structs
			// TODO: Camera and Light in Shaders
			// TODO: Use some Textured objects

			for (const auto& instance : objInstance.second) {
				glm::mat4 model = instance.translation * glm::mat4_cast(instance.rotation) * instance.scale;
				glm::mat4 modelViewProj = projection * view * model;
				int modelLoc = glGetUniformLocation(programs[currentProgram], "modelViewProj");
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelViewProj));
				glDrawElements(GL_TRIANGLES, scene->objects[objInstance.first].indices.size(), GL_UNSIGNED_INT, 0);
			}
			glBindVertexArray(0);
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
	}

	void OpenGLCore::createProgram() {
		try {
			programs.push_back(MirielEngine::OpenGL::createShaderProgram("1.vert", "1.frag"));
		} catch (const MirielEngine::Errors::OpenGLUtilError& e) {
			throw MirielEngine::Errors::OpenGLError(e.what());
		}
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
}
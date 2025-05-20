#pragma once

#include <string>
#include <memory>

#include <assimp/scene.h>

#include "Objects.hpp"


namespace MirielEngine::Core {
	void loadObject(const std::string& objectName, MirielEngine::Core::Object* object, const TextureLoadFunction& textureLoader);
	void processNode(aiNode* node, const aiScene* scene, MirielEngine::Core::Object* object, const TextureLoadFunction& textureLoader);
	void processMesh(aiMesh* mesh, const aiScene* scene, MirielEngine::Core::Object* object, const TextureLoadFunction& textureLoader);
	void loadMaterials(aiMaterial* material, aiTextureType type, std::string typeName, MirielEngine::Core::Object* object, const TextureLoadFunction& textureLoader);

	// TODO: Compress Texture Function
	void compressTexture(const std::string& textureName);
}
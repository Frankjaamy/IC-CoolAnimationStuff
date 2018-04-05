#pragma once
#include "ynMesh.h"
#include "assimp/scene.h"
using namespace CYN;
class ynModel final
{
public:
	ynModel(const char * modelFile) {
		loadModel(modelFile);
	}
	~ynModel();
public:
	void draw();
	void initOpenGL();
private:
	void loadModel(const char * modelFile);
	void processNode(aiNode * node, const aiScene * scene);
	Mesh processMesh(aiMesh * mesh, const aiScene * scene);
	std::vector<Texture> loadMatTextures(aiMaterial * mat, aiTextureType texType, std::string typeName);
public:
	std::vector<Mesh> m_vMeshes_;
	std::vector<Texture> m_vTexLoaded;
};


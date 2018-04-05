#include "ynModel.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include "ynMeshLoader.h"
ynModel::~ynModel()
{
}

void ynModel::loadModel(const char * modelFile)
{
	Assimp::Importer importer;
	const aiScene * scene = importer.ReadFile(modelFile, aiProcess_Triangulate);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Error Assimp: " << importer.GetErrorString() << std::endl;
		return;
	}
	processNode(scene->mRootNode, scene);
}

void ynModel::draw()
{
	for (auto iter : m_vMeshes_)
	{
		iter.draw();
	}
}

void ynModel::processNode(aiNode * node, const aiScene * scene)
{
	if (node == nullptr) return;

	for (unsigned i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh * mesh = scene->mMeshes[node->mMeshes[i]];
		m_vMeshes_.push_back(processMesh(mesh, scene));
	}
	for (unsigned i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}

	return;
}

CYN::Mesh ynModel::processMesh(aiMesh * mesh, const aiScene * scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;
	std::vector<Texture> textures;
	
	for (unsigned i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex oneVertex;
		oneVertex.position.x = mesh->mVertices[i].x;
		oneVertex.position.y = mesh->mVertices[i].y;
		oneVertex.position.z = mesh->mVertices[i].z;

		oneVertex.normal.x = mesh->mNormals[i].x;
		oneVertex.normal.y = mesh->mNormals[i].y;
		oneVertex.normal.z = mesh->mNormals[i].z;

		if (mesh->mTextureCoords[0]) {
			oneVertex.texCoord.x = mesh->mTextureCoords[0][i].x;
			oneVertex.texCoord.y = mesh->mTextureCoords[0][i].y;
			oneVertex.texCoord.z = mesh->mTextureCoords[0][i].z;
		}
		vertices.push_back(oneVertex);
	}
	for (unsigned i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace * face = &mesh->mFaces[i];
		for (unsigned j = 0; j < face->mNumIndices; j++)
		{
			indices.push_back(face->mIndices[j]);
		}
	}
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial * mat = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = loadMatTextures(mat, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<Texture> specularMaps = loadMatTextures(mat, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}
	return Mesh(vertices, textures, indices, mesh);
}

std::vector<CYN::Texture> ynModel::loadMatTextures(aiMaterial * mat, aiTextureType texType, std::string typeName)
{
	std::vector<Texture> textures;
	for (unsigned i = 0; i < mat->GetTextureCount(texType); i++)
	{
		aiString str;
		mat->GetTexture(texType, i, &str);
		for (auto iter : m_vTexLoaded)
		{
			if (strcmp(str.C_Str(), iter.path.c_str()) == 0)
			{
				textures.push_back(iter);
				continue;
			}
		}	
		Texture tex;
		if (!ModelLoader::ModelLoaderBase::loadTexture(tex, str.C_Str())) {
			assert(false);
		}
		tex.path = std::string(str.C_Str());
		m_vTexLoaded.push_back(tex);
		textures.push_back(tex);
	}
	return textures;
}

void ynModel::initOpenGL()
{
	for (unsigned i = 0; i < m_vMeshes_.size(); i++)
	{
		m_vMeshes_[i].initOpenGL();
	}
}


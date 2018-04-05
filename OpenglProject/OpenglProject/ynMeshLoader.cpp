#include "ynMeshLoader.h"
#include "TextureLoader/lodepng.h"
#include <assert.h>
using namespace CYN;
using namespace CYN::ModelLoader;
Mesh * ModelLoaderFactory::loadMeshFromFile(const char * fileName, std::string fileType)
{
	if (strcmp(fileType.c_str(), "cyTriMesh") == 0)
	{
		CyModelLoader loader;
		return loader.loadMeshFromFile(fileName);
	}
}

Mesh * CyModelLoader::loadMeshFromFile(const char * fileName)
{
	if (strlen(fileName) == 0 || fileName == nullptr) return nullptr;

	cy::TriMesh * loader = new cy::TriMesh();
	if (!loader->LoadFromFileObj(fileName)) return nullptr;

	unsigned int numOfVerticesNeeded = loader->NF() * 3;
	glm::vec3 * vertexDataBuffer = nullptr;
	glm::vec3 * normalDataBuffer = nullptr;
	glm::vec3 * uvDataBuffer = nullptr;

	buildDataBuffer(numOfVerticesNeeded, *loader, vertexDataBuffer, normalDataBuffer, uvDataBuffer);
	std::vector<Vertex> vertexData;
	std::vector<Texture> texData;
	for (size_t i = 0; i < numOfVerticesNeeded; i++)
	{
		Vertex vertex;
		vertex.position = vertexDataBuffer[i];
		vertex.normal = normalDataBuffer[i];
		vertex.texCoord = uvDataBuffer[i];
		vertexData.push_back(vertex);
	}
	Texture tex;
	if (loadTexture(tex, loader->M(0).map_Kd.data)) {
		texData.push_back(tex);
	}
	Texture tex1;
	if (loadTexture(tex1, loader->M(0).map_Ks.data)) {
		texData.push_back(tex1);
	}
	return new Mesh(vertexData, texData, loader);
}

void CyModelLoader::buildDataBuffer(unsigned int numOfVerticesNeeded, const cyTriMesh & mesh, glm::vec3 * & vertexData, glm::vec3 * & normalData, glm::vec3 * & uvData)
{
	vertexData = new glm::vec3[numOfVerticesNeeded];
	normalData = new glm::vec3[numOfVerticesNeeded];
	uvData = new glm::vec3[numOfVerticesNeeded];
	for (unsigned int i = 0; i < numOfVerticesNeeded; i++)
	{
		unsigned int vertexIndex = mesh.F(i / 3).v[i % 3];
		unsigned int normalIndex = mesh.FN(i / 3).v[i % 3];
		unsigned int uvIndex = mesh.FT(i / 3).v[i % 3];

		vertexData[i] = convert_from_cyPoint3f(mesh.V(vertexIndex));
		normalData[i] = convert_from_cyPoint3f(mesh.VN(normalIndex));
		uvData[i] = convert_from_cyPoint3f(mesh.VT(uvIndex));
	}
}

bool CYN::ModelLoader::ModelLoaderBase::loadTexture(Texture & o_Texture, const char * i_texFile)
{
	uint8_t * data = nullptr;
	unsigned width;
	unsigned height;
	if (lodepng_decode32_file(&data, &width, &height, i_texFile))
	{
		assert("load texture fail");
		return false;
	}
	o_Texture.type = GL_TEXTURE_2D;
	glGenTextures(1, &o_Texture.id);

	glBindTexture(o_Texture.type, o_Texture.id);
	glTexImage2D(o_Texture.type, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)data);
	glTexParameterf(o_Texture.type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(o_Texture.type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return true;
}
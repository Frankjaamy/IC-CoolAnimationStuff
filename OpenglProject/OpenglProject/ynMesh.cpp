#include "ynMesh.h"

using namespace CYN;

void Mesh::initOpenGL()
{
	assert(m_vVertices.size() > 0 && m_vTextures.size() > 0);
	glGenVertexArrays(1, &m_gliVao);
	glBindVertexArray(m_gliVao);

	size_t numOfVerticesNeeded = m_vVertices.size();
	glm::vec3 * vertexData = new glm::vec3[numOfVerticesNeeded];
	for (size_t i = 0; i < numOfVerticesNeeded; i++)
	{
		vertexData[i] = m_vVertices[i].position;
	}
	glGenBuffers(1, &m_gliVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_gliVbo);
	glBufferData(GL_ARRAY_BUFFER, numOfVerticesNeeded * sizeof(glm::vec3), vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	delete [] vertexData;

	glm::vec3 * normalData = new glm::vec3[numOfVerticesNeeded];
	for (size_t i = 0; i < numOfVerticesNeeded; i++)
	{
		normalData[i] = m_vVertices[i].normal;
	}
	glGenBuffers(1, &m_gliNbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_gliNbo);
	glBufferData(GL_ARRAY_BUFFER, numOfVerticesNeeded * sizeof(cyPoint3f), normalData, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glm::vec3 * uvData = new glm::vec3[numOfVerticesNeeded];
	for (size_t i = 0; i < numOfVerticesNeeded; i++)
	{
		uvData[i] = m_vVertices[i].texCoord;
	}
	glGenBuffers(1, &m_gliUvbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_gliUvbo);
	glBufferData(GL_ARRAY_BUFFER, numOfVerticesNeeded * sizeof(cyPoint3f), uvData, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 12, 0);
	glEnableVertexAttribArray(2);

	if (m_vIndices.size() > 0)
	{
		glGenBuffers(1, &m_gliIndiceBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gliIndiceBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vIndices.size() * sizeof(unsigned), &m_vIndices[0], GL_STATIC_DRAW);
	}
}

CYN::Mesh::Mesh(const std::vector<Vertex> & i_verticesData, const std::vector<Texture> & i_texData, cyTriMesh * i_loader)
{
	m_pMeshLoader = i_loader;
	m_vVertices = i_verticesData;
	m_vTextures = i_texData;
}

CYN::Mesh::Mesh(const std::vector<Vertex> & i_verticesData, const std::vector<Texture> & i_texData, const std::vector<unsigned> & i_indices, aiMesh * i_mesh):m_pMeshLoader(nullptr),m_pAiMesh(i_mesh)
{
	m_vVertices = i_verticesData;
	m_vTextures = i_texData;
	m_vIndices = i_indices;
}

void CYN::Mesh::draw()
{
	if (m_pMeshLoader) {
		glBindVertexArray(m_gliVao);
		glDrawArrays(GL_TRIANGLES, 0, m_pMeshLoader->NF() * 3);
	}
	else 
	{
		glBindVertexArray(m_gliVao);
		glDrawArrays(GL_POINT, 0, 15);

	}
}

CYN::Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &m_gliVao);
	if (m_pMeshLoader) {
		m_pMeshLoader->Clear();
		delete m_pMeshLoader;
	}
}

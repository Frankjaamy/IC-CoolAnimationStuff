#pragma once
#include <vector>
#include <string>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#include "GL/glew.h"
#include "GL/freeglut.h"

#include "cy/cyMatrix.h"
#include "cy/cyTriMesh.h"
#include "cy/cyGL.h"

#include "assimp/mesh.h"
namespace CYN
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 texCoord;
	};
	struct Texture
	{
		GLuint id;
		GLuint type;
		std::string path;
	};
	class Mesh
	{
	public:
		void initOpenGL();
		void draw();
	public:
		cyTriMesh * m_pMeshLoader;
		aiMesh * m_pAiMesh;
		std::vector<Vertex> m_vVertices;
		std::vector<Texture> m_vTextures;
		std::vector<unsigned> m_vIndices;
		Mesh(const std::vector<Vertex> & i_verticesData, const std::vector<Texture> & i_texData, cyTriMesh * i_loader = nullptr);
		Mesh(const std::vector<Vertex> & i_verticesData, const std::vector<Texture> & i_texData, const std::vector<unsigned> & i_indices, aiMesh * i_mesh = nullptr);
		~Mesh();
	public:
		inline GLuint getVAO() const { return m_gliVao; }
	private:
		GLuint m_gliVao, m_gliVbo, m_gliNbo, m_gliUvbo;
		GLuint m_gliIndiceBuffer;
	};
}

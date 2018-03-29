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
	};
	class Mesh
	{
	public:
		void initOpenGL();
		void draw();
	public:
		cyTriMesh * loader_;
		std::vector<Vertex> vertices_;
		std::vector<Texture> textures_;
		Mesh(const std::vector<Vertex> & i_verticesData, const std::vector<Texture> i_texData, cyTriMesh * i_loader = nullptr);
		~Mesh();
	public:
		inline GLuint getVAO() const { return vao_; }
	private:
		GLuint vao_, vbo_, nbo_, uvbo_;

	};
	namespace MeshLoader
	{
		class MeshLoaderBase
		{
		public:
			virtual Mesh * loadMeshFromFile(const char * fileName) = 0;
			bool loadTexture(Texture & o_Texture, const char * i_texFile);
		};

		class CyMeshLoader : public MeshLoaderBase
		{
		public:
			virtual Mesh * loadMeshFromFile(const char * fileName);
		private:
			void buildDataBuffer(unsigned int numOfVerticesNeeded, const cyTriMesh & mesh, glm::vec3 * & vertexData, glm::vec3 * & normalData, glm::vec3 * & uvData);
			inline glm::vec3 convert_from_cyPoint3f(const cy::Point3f & point)
			{
				glm::vec3 outVec;
				outVec.x = point.x;
				outVec.y = point.y;
				outVec.z = point.z;
				return outVec;
			}
		};

		class MeshLoaderFactory
		{
		public:
			static Mesh * loadMeshFromFile(const char * fileName, std::string fileType);
		};
	}
}

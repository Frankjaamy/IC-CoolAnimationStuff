#pragma once
#include "ynMesh.h"
namespace CYN {
	namespace ModelLoader
	{
		class ModelLoaderBase
		{
		public:
			virtual Mesh * loadMeshFromFile(const char * fileName) = 0;
			static bool loadTexture(Texture & o_Texture, const char * i_texFile);
		};

		class CyModelLoader : public ModelLoaderBase
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

		class ModelLoaderFactory
		{
		public:
			static Mesh * loadMeshFromFile(const char * fileName, std::string fileType);
		};
	}
}
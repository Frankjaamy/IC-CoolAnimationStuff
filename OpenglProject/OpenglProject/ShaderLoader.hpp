#include "GL/glew.h"
#include "GL/freeglut.h"
#include "ynMesh.h"

namespace ShaderLoader
{
	/* This function is from Wiki. Somehow the professor's shader loading library is not working.
	/* A simple function that will read a file into an allocated char pointer buffer */
	char* filetobuf(char *file)
	{
		FILE *fptr;
		long length;
		char *buf;

		fptr = fopen(file, "rb"); /* Open file for reading */
		if (!fptr) /* Return NULL on failure */
			return NULL;
		fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
		length = ftell(fptr); /* Find out how many bytes into the file we are */
		buf = (char*)malloc(length + 1); /* Allocate a buffer for the entire length of the file and a null terminator */
		fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
		fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
		fclose(fptr); /* Close the file */
		buf[length] = 0; /* Null terminator */

		return buf; /* Return the buffer */
	}
	bool loadShader(char * shaderVertexFile, char * shaderFragFile, GLuint & o_shaderProgram)
	{
		static char * vertexSource = nullptr;
		static char * fragmentSource = nullptr;
		if (vertexSource)
			free(vertexSource);
		if (fragmentSource)
			free(fragmentSource);


		int IsCompiled_VS, IsCompiled_FS;
		int IsLinked;

		std::string vertexShaderFullDir("Shaders/");
		vertexShaderFullDir.append(shaderVertexFile);
		vertexSource = filetobuf(const_cast<char *>(shaderVertexFile));

		std::string vertexFragFullDir("Shaders/");
		vertexFragFullDir.append(shaderVertexFile);
		fragmentSource = filetobuf(const_cast<char *>(shaderFragFile));

		GLuint vertexshader = glCreateShader(GL_VERTEX_SHADER);

		glShaderSource(vertexshader, 1, (const GLchar**)&vertexSource, 0);
		glCompileShader(vertexshader);

		glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &IsCompiled_VS);
		if (IsCompiled_VS == FALSE)
		{
			return false;
		}

		GLuint fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentSource, 0);
		glCompileShader(fragmentshader);

		glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &IsCompiled_FS);
		if (IsCompiled_FS == FALSE)
		{
			return false;
		}

		o_shaderProgram = glCreateProgram();

		glAttachShader(o_shaderProgram, vertexshader);
		glAttachShader(o_shaderProgram, fragmentshader);

		glLinkProgram(o_shaderProgram);

		glGetProgramiv(o_shaderProgram, GL_LINK_STATUS, (int *)&IsLinked);
		if (IsLinked == FALSE)
		{
			return false;
		}
		return true;
	}
}
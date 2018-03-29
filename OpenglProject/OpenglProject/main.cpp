#include <chrono>
#include <ctime>
#include <math.h>
#include <random>	

#include "GL/glew.h"
#include "GL/freeglut.h"

#include "TextureLoader/lodepng.h"

#include "cy/cyMatrix.h"
#include "cy/cyTriMesh.h"
#include "cy/cyGL.h"

#include "glm\matrix.hpp"
#include "glm\gtc\matrix_transform.hpp"


const float PI = 3.1415926f;

struct Texture
{
	GLuint textureId;
	GLuint textureObjectId;
};

namespace Parameters 
{
	float light[3] = { -3.0,0.0f,-5.0f };
	cy::Point3f lightRotAxis(0, 1, 0);
	float lightRotAngle = 0.0f;
	cy::Matrix4<float> curLightRotMatrixCam = cy::Matrix4<float>::MatrixRotation(cy::Point3f(1.0, 0.0, 0.0), 0);

	int windowId;
	GLclampf oldR = 0.0f, oldG = 0.0f, oldB = 0.0f;
	GLclampf newR = 1.0f, newG = 1.0f, newB = 1.0f;
	float timeUsed = 0.0f;
	GLuint verBufObject;
	GLuint verArrayObject;

	cy::TriMesh panelLoader;
	cy::TriMesh teapotLoader;

	Texture tex1;
	Texture tex2;

	GLuint vao1, vbo1, nbo1, uvbo1;
	GLuint vao2, vbo2, nbo2, uvbo2;

	GLuint FramebufferName = 0;
	GLuint renderedTexture;

	namespace Teapot 
	{
		GLuint texture0Location;
		GLuint texture1Location;
		GLuint renderTextureLocation;

		GLuint lightDirection;
		GLuint cameraPosition;
		GLuint depthMap;
	}

	namespace Panel
	{
		GLuint texture0Location;
		GLuint texture1Location;
	}

	namespace ShapeData
	{
		static const GLfloat quadVertexData[] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f,  1.0f, 0.0f,
		};
	}
	namespace Shader 
	{
		GLuint shaderTeapot;
		GLuint shaderShadow;
		GLchar *vertexsource = nullptr, * fragmentsource = nullptr;
		GLuint vertexshader, fragmentshader;
	}
	enum eMouseMode 
	{
		edefaultMode,
		eRotateMode,
		eZoomMode,
		eLightMode
	} curMouseState;
	namespace ViewPort 
	{
		bool bProspective = true;
		float cameraDistance = 20.0f;
		cy::Point3f cameraRotAxis;
		cy::Matrix4<float> prevRotMatrixCam = cy::Matrix4<float>::MatrixRotation(cy::Point3f(1.0, 0.0, 0.0), 0);
		cy::Matrix4<float> curRotMatrixCam = cy::Matrix4<float>::MatrixRotation(cy::Point3f(1.0, 0.0, 0.0), 0);
		cy::Matrix4<float> viewPort;
		float cameraRotAngle = 0.0f;
		float lastX = 0, lastY = 0;
	}
}


namespace MathFunc
{
	float lerp(float a, float b, float percentage)
	{
		if (percentage > 1.0f) percentage = 1.0f;
		if (percentage < 0.0f) percentage = 0.0f;
		return a * (1 - percentage) + b*percentage;
	}
}
namespace UtilityFunc
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
		using namespace Parameters::Shader;

		if(vertexsource)
			free(vertexsource);
		if(fragmentsource)
			free(fragmentsource);


		int IsCompiled_VS, IsCompiled_FS;
		int IsLinked;

		std::string vertexShaderFullDir("Shaders/");
		vertexShaderFullDir.append(shaderVertexFile);
		vertexsource = UtilityFunc::filetobuf(const_cast<char *>(shaderVertexFile));
		
		std::string vertexFragFullDir("Shaders/");
		vertexFragFullDir.append(shaderVertexFile);
		fragmentsource = UtilityFunc::filetobuf(const_cast<char *>(shaderFragFile));

		vertexshader = glCreateShader(GL_VERTEX_SHADER);

		glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);

		glCompileShader(vertexshader);

		glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &IsCompiled_VS);
		if (IsCompiled_VS == FALSE)
		{
			return false;
		}

		fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);

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

	bool loadTexture(Texture & o_Texture, const char * i_textureFile)
	{
		uint8_t * data = nullptr;
		unsigned width;
		unsigned height;
		if (lodepng_decode32_file(&data, &width, &height, i_textureFile))
		{
			assert("load texture fail");
			return false;
		}
		o_Texture.textureId = GL_TEXTURE_2D;
		glGenTextures(1, &o_Texture.textureObjectId);

		glBindTexture(o_Texture.textureId, o_Texture.textureObjectId);
		glTexImage2D(o_Texture.textureId, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)data);
		glTexParameterf(o_Texture.textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(o_Texture.textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		return true;
	}
}

using namespace Parameters;
namespace BindingFunctions
{
	void renderScene(void)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(Shader::shaderShadow);
		glBindVertexArray(Parameters::vao1);
		glDrawArrays(GL_TRIANGLES, 0, Parameters::panelLoader.NF() * 3);
		glBindVertexArray(Parameters::vao2);
		glDrawArrays(GL_TRIANGLES, 0, Parameters::teapotLoader.NF() * 3);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
#if 1
		glUseProgram(Shader::shaderTeapot);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex1.textureObjectId);
		glUniform1i(Teapot::texture0Location, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex2.textureObjectId);
		glUniform1i(Teapot::texture1Location, 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, renderedTexture);
		Panel::texture0Location = glGetUniformLocation(Shader::shaderTeapot, "shadowMap");
		glUniform1i(Panel::texture0Location, 2);

		glBindVertexArray(Parameters::vao1);
		glDrawArrays(GL_TRIANGLES, 0, Parameters::panelLoader.NF() * 3);

		glUseProgram(Shader::shaderTeapot);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex1.textureObjectId);
		glUniform1i(Teapot::texture0Location, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex2.textureObjectId);
		glUniform1i(Teapot::texture1Location, 1);

		glBindVertexArray(Parameters::vao2);
		glDrawArrays(GL_TRIANGLES, 0, Parameters::teapotLoader.NF() * 3);
#endif
		glutSwapBuffers();

	}

	void keyFunc(unsigned char keyChar, int x, int y)
	{
		switch (keyChar)
		{
		case 27: // escape
		{
			glutDestroyWindow(windowId);
			exit(0);
		}
		break;
		case 'w':
		{
			Parameters::light[1] += 0.15f;
		}
		break;
		case 's':
		{
			Parameters::light[1] -= 0.15f;
		}
		break;
		case 'a':
		{
			Parameters::light[0] -= 0.15f;
		}
		break;
		case 'd':
		{
			Parameters::light[0] += 0.15f;
		}
		break;
		default:
		break;
		}
	}

	void postRender(void)
	{
		using namespace Parameters::ViewPort;
		GLint camId = glGetUniformLocation(Shader::shaderTeapot, "cameraMatrix");

		cy::Matrix4<float> scaleMatrixObject = cy::Matrix4<float>::MatrixScale(cy::Point3f(0.2f, 0.2f, 0.2f));
		cy::Matrix4<float> rotMatrixObject = cy::Matrix4<float>::MatrixRotation(cy::Point3f(0.0f, 1.0f, 0.0f), 0.0f);
		cy::Matrix4<float> transMatrixObject = cy::Matrix4<float>::MatrixTrans(cy::Point3f(0.0f, 0.0f, 0.0f));

		cy::Matrix4<float> scaleMatrixCam = cy::Matrix4<float>::MatrixScale(cy::Point3f(1, 1, 1));
		curRotMatrixCam = cy::Matrix4<float>::MatrixRotation(cameraRotAxis, -cameraRotAngle);
		cy::Matrix4<float> rotMatrixCam = prevRotMatrixCam * curRotMatrixCam;

		cy::Point4f cameraForward(0, 0, 1.0f, 0.0f);
		cameraForward = rotMatrixCam * cameraForward;
		cameraForward.Normalize();
		cy::Matrix4<float> transMatrixCam = cy::Matrix4<float>::MatrixTrans(cy::Point3f(cameraForward.x * cameraDistance, cameraForward.y * cameraDistance, cameraForward.z * cameraDistance));
		cy::Matrix4<float> worldToCameraMatrix = (transMatrixCam * rotMatrixCam * scaleMatrixCam).GetInverse();
		viewPort = cy::Matrix4<float>::MatrixPerspective(PI / 4, 4 / 3.0f, 0.1f, 100.0f);

		cy::Matrix4<float> objectToWorldMatrix = viewPort * worldToCameraMatrix *transMatrixObject * rotMatrixObject * scaleMatrixObject;
		glUniformMatrix4fv(camId, 1, GL_FALSE, objectToWorldMatrix.data);

		cyPoint3f cameraPos = cy::Point3f(cameraForward.x * cameraDistance, cameraForward.y * cameraDistance, cameraForward.z * cameraDistance);

		cy::Matrix4<float> lightRot = cy::Matrix4<float>::MatrixRotation(lightRotAxis, -lightRotAngle);
		cy::Point4f lightCY(light[0], light[1], light[2], 1);

		lightCY = lightRot * lightCY;
		printf("%f, %f, %f\n", lightCY.x, lightCY.y, lightCY.z);

		glUniform3fv(Teapot::lightDirection, 1, &lightCY.x);
		glUniform3fv(Teapot::cameraPosition, 1, &cameraPos.x);


		glUseProgram(Shader::shaderShadow);
		GLint depthMatrixID = glGetUniformLocation(Shader::shaderShadow, "depthMVP");
		glm::vec3 lightInvDir = glm::vec3(-lightCY.x, lightCY.y, -lightCY.z);
		glm::vec3 glm_cameraPos = glm::vec3(cameraPos.x, cameraPos.y, cameraPos.z);

		// Compute the MVP matrix from the light's point of view
		glm::mat4 depthProjectionMatrix = glm::ortho<float>(-20, 20, -20, 20, -20, 20);
		glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0, 1, 0));
		glm::mat4 depthModelMatrix = glm::mat4(1.0);
		glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &depthMVP[0][0]);

		//use to transfer [-1,1] to [0, 1]
		glm::mat4 biasMatrix(
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0
		);
		glm::mat4 depthBiasMVP = biasMatrix*depthMVP;
		glUseProgram(Shader::shaderTeapot);
		GLint biasMVPId = glGetUniformLocation(Shader::shaderTeapot, "biasMVP");
		glUniformMatrix4fv(biasMVPId, 1, GL_FALSE, &depthBiasMVP[0][0]);

		glutPostRedisplay();
		return;
	}

	void mouseMotionFunc(int x, int y)
	{
		using namespace Parameters::ViewPort;
		switch (curMouseState)
		{
			case eRotateMode:
			{
				cy::Point3f v1((float)x, (float)y, 0);
				cy::Point3f v2(lastX, lastY, 0);

				cy::Point3f v3 = v2 - v1;
				cameraRotAngle = v3.Length()/100;
				v3.Normalize();
				cy::Point3f v4(0, 0, -1.0f);

				cameraRotAxis = v3.Cross(v4);
			}
				break;
			case eLightMode:
			{
				cy::Point3f v1((float)x, (float)y, 0);
				cy::Point3f v2(lastX, lastY, 0);

				cy::Point3f v3 = v2 - v1;
				lightRotAngle = v3.Length() / 200;
				v3.Normalize();
				cy::Point3f v4(0, 0, 1.0f);

				lightRotAxis = v3.Cross(v4);

			}
			break;

			case eZoomMode:
			{
				cy::Point3f v1((float)x, (float)y, 0);
				cy::Point3f v2(lastX, lastY, 0);

				cy::Point3f v3 = v2 - v1;

				int direction = v1.y > v2.y ?-1:1;
				cameraDistance += v3.Length() / 1000 * direction;
			}
				break;
		}

	}

	void mouseClickFunc(int mouse, int state, int x, int y)
	{
		using namespace Parameters::ViewPort;
		if (state == GLUT_DOWN && mouse == GLUT_LEFT_BUTTON) {
			curMouseState = eRotateMode;
		}
		else if (state == GLUT_DOWN && mouse == GLUT_RIGHT_BUTTON) {
			curMouseState = eZoomMode;
		}
		else if (state == GLUT_DOWN && mouse == GLUT_MIDDLE_BUTTON) {
			curMouseState = eLightMode;
		}
		else
		{
			curMouseState = edefaultMode;
			//prevRotMatrixCam = curRotMatrixCam;
		}
		lastX = (float)x;
		lastY = (float)y;
	}
}

void buildDataBuffer(unsigned int numOfVerticesNeeded, const cyTriMesh & mesh, cyPoint3f * & vertexData, cyPoint3f * & normalData, cyPoint3f * & uvData)
{
	vertexData = new cyPoint3f[numOfVerticesNeeded];
	normalData = new cyPoint3f[numOfVerticesNeeded];
	uvData = new cyPoint3f[numOfVerticesNeeded];
	for (unsigned int i = 0; i < numOfVerticesNeeded; i++)
	{
		unsigned int vertexIndex = mesh.F(i / 3).v[i % 3];
		unsigned int normalIndex = mesh.FN(i / 3).v[i % 3];
		unsigned int uvIndex = mesh.FT(i / 3).v[i % 3];

		vertexData[i] = mesh.V(vertexIndex);
		normalData[i] = mesh.VN(normalIndex);
		uvData[i] = mesh.VT(uvIndex);
	}
}

void buildOpenGLBuffer(unsigned int numOfVerticesNeeded, GLuint & vao, GLuint & vbo, GLuint & nbo, GLuint & uvbo, cyPoint3f * vertexData, cyPoint3f * normalData, cyPoint3f * uvData)
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#if 1
	{
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, numOfVerticesNeeded * sizeof(cyPoint3f), vertexData, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &nbo);
		glBindBuffer(GL_ARRAY_BUFFER, nbo);
		glBufferData(GL_ARRAY_BUFFER, numOfVerticesNeeded * sizeof(cyPoint3f), normalData, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &uvbo);
		glBindBuffer(GL_ARRAY_BUFFER, uvbo);
		glBufferData(GL_ARRAY_BUFFER, numOfVerticesNeeded * sizeof(cyPoint3f), uvData, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 12, 0);
		glEnableVertexAttribArray(2);
	}
#endif
}

int initWindow(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);

	//	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(1024, 768);
	windowId = glutCreateWindow("Haha");
	glClearColor(0, 0, 0, 1);

	glutDisplayFunc(BindingFunctions::renderScene);
	glutIdleFunc(BindingFunctions::postRender);
	glutKeyboardFunc(BindingFunctions::keyFunc);
	glutMotionFunc(BindingFunctions::mouseMotionFunc);
	glutMouseFunc(BindingFunctions::mouseClickFunc);

	GLenum result = glewInit();
	if (result != GLEW_OK)
	{
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(result));
		return 1;
	}
}
int main(int argc, char *argv[])
{
	initWindow(argc, argv);
	panelLoader.LoadFromFileObj("panel.obj");
	unsigned int numOfVerticesNeeded = panelLoader.NF() * 3;
	cyPoint3f * vertexDataPanel = nullptr;
	cyPoint3f * normalDataPanel = nullptr;
	cyPoint3f * uvDataPanel = nullptr;

	buildDataBuffer(numOfVerticesNeeded, panelLoader, vertexDataPanel, normalDataPanel, uvDataPanel);
	for (int i = 0; i < numOfVerticesNeeded; i++)
	{
		vertexDataPanel[i] *= 5.0f;
	}
	buildOpenGLBuffer(numOfVerticesNeeded, vao1, vbo1, nbo1, uvbo1, vertexDataPanel, normalDataPanel, uvDataPanel);
	
	teapotLoader.LoadFromFileObj("teapot.obj");
	numOfVerticesNeeded = teapotLoader.NF() * 3;

	cyPoint3f * vertexDataTeapot = nullptr;
	cyPoint3f * normalDataTeapot = nullptr;
	cyPoint3f * uvDataTeapot = nullptr;

	buildDataBuffer(numOfVerticesNeeded, teapotLoader, vertexDataTeapot, normalDataTeapot, uvDataTeapot);
	buildOpenGLBuffer(numOfVerticesNeeded, vao2, vbo2, nbo2, uvbo2, vertexDataTeapot, normalDataTeapot, uvDataTeapot);

	if (!UtilityFunc::loadTexture(tex1, panelLoader.M(0).map_Kd.data))
	{
		return 0;
	}
	if (!UtilityFunc::loadTexture(tex2, panelLoader.M(0).map_Ks.data))
	{
		return 0;
	}
	if (!UtilityFunc::loadShader("vertexShader.vert", "fragShader.frag", Shader::shaderTeapot))
	{
		return 0;
	}

	if (!UtilityFunc::loadShader("vertexPanel.vert", "fragPanel.frag", Shader::shaderShadow))
	{
		return 0;
	}
#if 1
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	glGenTextures(1, &Parameters::renderedTexture);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, renderedTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return 1;
	}
#endif
	{
		Teapot::texture0Location = glGetUniformLocation(Shader::shaderTeapot, "texSampler0");
		Teapot::texture1Location = glGetUniformLocation(Shader::shaderTeapot, "texSampler1");

		Teapot::lightDirection = glGetUniformLocation(Shader::shaderTeapot, "lightDirection");
		Teapot::cameraPosition = glGetUniformLocation(Shader::shaderTeapot, "viewPosition");

		Teapot::depthMap = glGetUniformLocation(Shader::shaderTeapot, "depthMap");

	}
	glutMainLoop();

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glDeleteProgram(Shader::shaderTeapot);
	return 0;
}
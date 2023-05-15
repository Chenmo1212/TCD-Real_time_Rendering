// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLM/gtx/euler_angles.hpp>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Project includes
// Loading photos
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "maths_funcs.h"
#define GLT_IMPLEMENTATION
#include "gltext.h"

#pragma region SimpleTypes
typedef struct
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
	std::vector<vec3> mTangents;
	std::vector<vec3> mBitangents;

} ModelData;
#pragma endregion SimpleTypes

using namespace std;

int width = 900;
int height = 900;

glm::vec3 lightPos, lightColor, objectColor, viewPos, cameraPos;
GLuint loc1, loc2, loc3, loc4, loc5;
GLfloat rotate_y = 0.0f;
GLfloat prop_rotate_y = 0.0f;
bool isCameraRotate = false;
GLfloat cameraRotateY = 0.0f;

glm::mat4 view, proj, model;

GLuint normalMappingProgramID, textShaderProgram, skyboxShaderProgramID;

const GLuint i = 15;
GLuint VAO[i], VBO[i * 2], VTO[i];
GLuint mappingMode = 1;
GLuint textureMode = 7;
GLuint modelType = 'f';
char* mappingName = "Diffuse Mapping";

std::vector < ModelData > meshData;
std::vector < const char* > dataArray;
ModelData mesh_data;

bool isFirstView = false;
bool isModelRotate = false;
float camera_dis = 4.5f;

// For debugging
float offsetX = 0.0f;
float offsetY = 0.0f;
float offsetZ = 0.0f;

#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name,
		aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_PreTransformVertices | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				const aiVector3D* vta = &(mesh->mTangents[v_i]);
				modelData.mTangents.push_back(vec3(vta->x, vta->y, vta->z));

				const aiVector3D* vbt = &(mesh->mBitangents[v_i]);
				modelData.mBitangents.push_back(vec3(vbt->x, vbt->y, vbt->z));
			}
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING

// ------------ SKYBOX ------------
unsigned int skyboxVAO, skyboxVBO;
unsigned int cubemapTexture;
vector<std::string> faces
{
	"./skybox/right.jpg",
	"./skybox/left.jpg",
	"./skybox/top.jpg",
	"./skybox/bottom.jpg",
	"./skybox/front.jpg",
	"./skybox/back.jpg"
};

float skyboxVertices[] = {
	-200.0f,  200.0f, -200.0f,
	-200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,
	 200.0f,  200.0f, -200.0f,
	-200.0f,  200.0f, -200.0f,

	-200.0f, -200.0f,  200.0f,
	-200.0f, -200.0f, -200.0f,
	-200.0f,  200.0f, -200.0f,
	-200.0f,  200.0f, -200.0f,
	-200.0f,  200.0f,  200.0f,
	-200.0f, -200.0f,  200.0f,

	 200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,

	-200.0f, -200.0f,  200.0f,
	-200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f, -200.0f,  200.0f,
	-200.0f, -200.0f,  200.0f,

	-200.0f,  200.0f, -200.0f,
	 200.0f,  200.0f, -200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	-200.0f,  200.0f,  200.0f,
	-200.0f,  200.0f, -200.0f,

	-200.0f, -200.0f, -200.0f,
	-200.0f, -200.0f,  200.0f,
	 200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,
	-200.0f, -200.0f,  200.0f,
	 200.0f, -200.0f,  200.0f
};

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}

unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int skyboxTextureID;
	glGenTextures(1, &skyboxTextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return skyboxTextureID;
}

void generateSkybox() {
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const char* vertexShaderName, const char* fragmentShaderName)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, vertexShaderName, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, fragmentShaderName, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh(std::vector < const char* > dataArray) {
	int width, height, nrChannels;
	unsigned char* data;
	int counter = 0;
	int texCounter = 0;

	loc1 = glGetAttribLocation(normalMappingProgramID, "vertex_position");
	loc2 = glGetAttribLocation(normalMappingProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(normalMappingProgramID, "vertex_texture");
	loc4 = glGetAttribLocation(normalMappingProgramID, "aTangent");
	loc5 = glGetAttribLocation(normalMappingProgramID, "aBitangent");

	for (int i = 0; i < dataArray.size(); i++) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, VTO[texCounter]);
		glUniform1i(glGetUniformLocation(normalMappingProgramID, "diffuseMap"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, VTO[texCounter + 1]);
		glUniform1i(glGetUniformLocation(normalMappingProgramID, "normalMap"), 1);
		mesh_data = load_mesh(dataArray[i]);
		meshData.push_back(mesh_data);

		glGenBuffers(1, &VBO[counter]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[counter]);
		glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &VBO[counter + 1]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[counter + 1]);
		glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);

		glGenBuffers(1, &VBO[counter + 2]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[counter + 2]);
		glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec2), &mesh_data.mTextureCoords[0], GL_STATIC_DRAW);

		glGenBuffers(1, &VBO[counter + 3]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[counter + 3]);
		glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mTangents[0], GL_STATIC_DRAW);

		glGenBuffers(1, &VBO[counter + 4]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[counter + 4]);
		glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mBitangents[0], GL_STATIC_DRAW);

		glGenVertexArrays(1, &VAO[i]);
		glBindVertexArray(VAO[i]);

		glEnableVertexAttribArray(loc1);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[counter]);
		glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glEnableVertexAttribArray(loc2);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[counter + 1]);
		glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glEnableVertexAttribArray(loc3);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[counter + 2]);
		glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		glEnableVertexAttribArray(loc4);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[counter + 3]);
		glVertexAttribPointer(loc4, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glEnableVertexAttribArray(loc5);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[counter + 4]);
		glVertexAttribPointer(loc5, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		counter += 5;
		texCounter += 2;
	}
}
// taken directly from https://learnopengl.com/Advanced-Lighting/Normal-Mapping
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad(glm::vec3 pos, float len)
{
	if (quadVAO == 0)
	{
		// positions
		glm::vec3 pos1(-len/2 + pos.x, len / 2 + pos.y, pos.z);
		glm::vec3 pos2(-len / 2 + pos.x, -len / 2 + pos.y, pos.z);
		glm::vec3 pos3(len / 2 + pos.x, -len / 2 + pos.y, pos.z);
		glm::vec3 pos4(len / 2 + pos.x, len / 2 + pos.y, pos.z);
		// texture coordinates
		glm::vec2 uv1(0.0f, 1.0f);
		glm::vec2 uv2(0.0f, 0.0f);
		glm::vec2 uv3(1.0f, 0.0f);
		glm::vec2 uv4(1.0f, 1.0f);
		// normal vector
		glm::vec3 nm(0.0f, 0.0f, 1.0f);

		// calculate tangent/bitangent vectors of both triangles
		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;
		// triangle 1
		// ----------
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		// triangle 2
		// ----------
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


		float quadVertices[] = {
			// positions            // normal         // texcoords  // tangent                          // bitangent
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
		};
		// configure plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

#pragma region TEXTURE_FUNCTIONS
unsigned int loadTexture(const char* texture, int i) {
	glGenTextures(1, &VTO[i]);
	int width, height, nrComponents;
	unsigned char* data = stbi_load(texture, &width, &height, &nrComponents, 0);

	GLenum format;

	cout << i << ":" << nrComponents << endl;
	if (nrComponents == 1)
		format = GL_RED;
	else if (nrComponents == 3)
		format = GL_RGB;
	else if (nrComponents == 4)
		format = GL_RGBA;

	glBindTexture(GL_TEXTURE_2D, VTO[i]);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);

	return VTO[i];
}

void drawText(const char* str, GLfloat size, glm::vec3 pos, glm::vec4 color=glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
	glUseProgram(textShaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "proj"), 1, GL_FALSE, glm::value_ptr(projection));

	// Initialize glText
	gltInit();

	// Creating text
	GLTtext* text = gltCreateText();
	gltSetText(text, str);

	// Begin text drawing (this for instance calls glUseProgram)
	gltBeginDraw();

	// Draw any amount of text between begin and end
	gltColor(color.x, color.y, color.z, color.w);
	gltDrawText2DAligned(text, 70 * (pos.x + 1), 450 - pos.y * 70, size, GLT_CENTER, GLT_CENTER);

	// Finish drawing text
	gltEndDraw();

	// Deleting text
	gltDeleteText(text);

	// Destroy glText
	gltTerminate();
	glDisable(GL_BLEND);
}

void drawSkybox() {
	glDepthFunc(GL_LEQUAL);
	glUseProgram(skyboxShaderProgramID);

	glUniformMatrix4fv(glGetUniformLocation(skyboxShaderProgramID, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(skyboxShaderProgramID, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
}

// This function is responsible for drawing 3D models on the screen.
// It takes in a vec3 position parameter to specify where to draw the model.
void drawModels(glm::vec3 pos) {

	// Use the shader program for normal mapping
	glUseProgram(normalMappingProgramID);

	// Translate the model to the given position
	model = glm::translate(model, pos);

	// If the model is a 't' (triangle), rotate it around the y-axis, and scale it down
	if (modelType == 't') {
		model = glm::rotate(model, glm::radians(rotate_y), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.6, 0.6, 0.6));
	}

	// Set the uniform variables for projection, view, and model matrices
	glUniformMatrix4fv(glGetUniformLocation(normalMappingProgramID, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(normalMappingProgramID, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(normalMappingProgramID, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// Set the uniform variables for light position and camera position
	glUniform3fv(glGetUniformLocation(normalMappingProgramID, "lightPos"), 1, &lightPos[0]);
	glUniform3fv(glGetUniformLocation(normalMappingProgramID, "viewPos"), 1, &cameraPos[0]);

	// Set the diffuse, normal, and depth maps based on the texture mode
	GLuint diffuseMap, normalMap, depthMap;
	if (textureMode == 7) {
		diffuseMap = VTO[0];
		normalMap = VTO[1];
		depthMap = VTO[2];
	}
	else if (textureMode == 8) {
		diffuseMap = VTO[3];
		normalMap = VTO[4];
		depthMap = VTO[5];
	}
	else if (textureMode == 9) {
		diffuseMap = VTO[6];
		normalMap = VTO[7];
		depthMap = VTO[8];
	}

	// Activate and bind the diffuse, normal, and depth maps to texture units 0, 1, and 2, respectively
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glUniform1i(glGetUniformLocation(normalMappingProgramID, "diffuseMap"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	glUniform1i(glGetUniformLocation(normalMappingProgramID, "normalMap"), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(glGetUniformLocation(normalMappingProgramID, "depthMap"), 2);

	// Set the mapping mode and UV scalar uniform variables
	glUniform1i(glGetUniformLocation(normalMappingProgramID, "mappingMode"), mappingMode);
	glUniform1i(glGetUniformLocation(normalMappingProgramID, "uvScalar"), 1);

	// Draw the model based on its type
	switch (modelType) {
	case 'f':
		renderQuad(pos, 2);
		break;
	case 's':
		break;
	case 't':
		glBindVertexArray(VAO[0]);
		glDrawArrays(GL_TRIANGLES, 0, meshData[0].mPointCount);
		break;
	}
}

void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.5f + cameraZ));
	view = glm::mat4(1.0f);
	float radian = glm::radians(cameraRotateY);
	view = glm::lookAt(glm::vec3(camera_dis * sin(radian), 0.0f, 0.0f + camera_dis * cos(radian)),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
	model = glm::mat4(1.0f);

	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	lightPos = glm::vec3(0.0f + offsetX, 0.0f + offsetY, 2.0f + offsetZ);

	// ------------------------------------------------- SKYBOX ------------------------------------------------- 
	drawSkybox();

	// ------------------------------------------------- TEXT ------------------------------------------------- 
	// Show Project Title
	drawText("Mapping Compare", 3, glm::vec3(5.4, 5.5, 0));

	char array[30];
	snprintf(array, sizeof(array), "LightX: %1.1f", offsetX);
	drawText(array, 2, glm::vec3(2.4f, -4.5f, 0.0f));
	snprintf(array, sizeof(array), "LightY: %1.1f", offsetY);
	drawText(array, 2, glm::vec3(5.4f, -4.5f, 0.0f));
	snprintf(array, sizeof(array), "LightZ: %1.1f", offsetZ);
	drawText(array, 2, glm::vec3(8.4f, -4.5f, 0.0f));

	snprintf(array, sizeof(array), "Name: %s", mappingName);
	drawText(array, 2, glm::vec3(5.4f, -3.5f, 0.0f), glm::vec4(237.0/255, 208.0/255, 2.0/255, 1));

	// ------------------------------------------------- MODEL ------------------------------------------------- 
	drawModels(glm::vec3(0, 0.1f, 0.0f));

	glutSwapBuffers();
}


void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	if (isModelRotate) {
		rotate_y += 10.0f * delta;
		rotate_y = fmodf(rotate_y, 360.0f);
	}

	if (isCameraRotate) {
		cameraRotateY += 10.0f * delta;
		cameraRotateY = fmodf(cameraRotateY, 360.0f);
	}

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Set up the shaders
	normalMappingProgramID = CompileShaders("./shaders/mappingVertexShader.txt", "./shaders/mappingFragmentShader.txt");
	textShaderProgram = CompileShaders("./shaders/textVertexShader.txt", "./shaders/textFragmentShader.txt");
	skyboxShaderProgramID = CompileShaders("./shaders/skyboxVertexShader.txt", "./shaders/skyboxFragmentShader.txt");

	// Skybox
	generateSkybox();
	cubemapTexture = loadCubemap(faces);

	// Textures
	unsigned int brickWallDiffuseMap = loadTexture("./Textures/brick_basecolor.jpg", 0);
	unsigned int brickWallNormalMap = loadTexture("./Textures/brick_normal.jpg", 1);
	unsigned int brickWallDepthMap = loadTexture("./Textures/brick_height.jpg", 2);

	unsigned int rockDiffuseMap = loadTexture("./Textures/rock_basecolor.jpg", 3);
	unsigned int rockNormalMap = loadTexture("./Textures/rock_normal.jpg", 4);
	unsigned int rockDepthMap = loadTexture("./Textures/rock_height.jpg", 5);

	unsigned int stoneDiffuseMap = loadTexture("./Textures/stone_basecolor.jpg", 6);
	unsigned int stoneNormalMap = loadTexture("./Textures/stone_normal.jpg", 7);
	unsigned int stoneDepthMap = loadTexture("./Textures/stone_height.jpg", 8);



	// load mesh into a vertex buffer array
	dataArray.push_back("./models/teapot.dae");

	//dataArray.push_back(TEAPOT_MESH);
	generateObjectBufferMesh(dataArray);
}

void keyPress(unsigned char key, int xmouse, int ymouse) {
	switch (key) {
		case('w'):
			offsetY += .5f;
			break;
		case('s'):
			offsetY -= .5f;
			break;
		case('a'):
			offsetX += .5f;
			break;
		case('d'):
			offsetX -= .5f;
			break;
		case('q'):
			offsetZ -= .5f;
			break;
		case('e'):
			offsetZ += .5f;
			break;
		case('r'):
			offsetX = 0.0f;
			offsetY = 0.0f;
			offsetZ = 0.0f;
			rotate_y = 0.0f;
			cameraRotateY = 0.0f;
			camera_dis = 4.5f;
			break;
		case('f'):
			modelType = 'f';
			break;
		case('t'):
			modelType = 't';
			break;
		case('m'):
			isModelRotate = !isModelRotate;
			break;
		case('c'):
			isCameraRotate = !isCameraRotate;
			break;
		case('1'):
			mappingMode = 1;
			mappingName = "Diffuse Mapping";
			break;
		case('2'):
			mappingMode = 2;
			mappingName = "Normal Mapping";
			break;
		case('3'):
			mappingMode = 3;
			mappingName = "Parallax Mapping";
			break;
		case('7'):
			textureMode = 7;
			break;
		case('8'):
			textureMode = 8;
			break;
		case('9'):
			textureMode = 9;
			break;
		}
	cout << offsetX << ", " << offsetY << ", " << offsetZ << ", " << mappingMode << endl;
};

void specialKeypress(int key, int x, int y) {
	switch (key) {
		// Specular Lighting settings, up for up, down for down
	case(GLUT_KEY_UP):
		offsetY += 0.2;
		break;
	case(GLUT_KEY_DOWN):
		offsetY -= 0.2;
		break;
	case(GLUT_KEY_LEFT):
		offsetX -= 0.2;
		break;
	case(GLUT_KEY_RIGHT):
		offsetX += 0.2;
		break;
	}
	cout << key << ": " << offsetX << ",   " << offsetY << ",   " << endl;
}

void mouseMove(int x, int y) {
	// Fix x_mouse coordinates calculation to only take the first viewport
	//x_mouse = (float)-(x - width / 2) / (width / 2);
	//y_mouse = (float)-(y - height / 2) / (height / 2);
};

void mouseWheel(int key, int wheeldir, int x, int y) {
	if (wheeldir == 1)
	{
		camera_dis -= 0.2f;
	}
	else if (wheeldir == -1) {
		camera_dis += 0.2f;
	}
};

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Lab 3 - Bump/Normal Mapping");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keyPress);
	glutSpecialFunc(specialKeypress);
	glutPassiveMotionFunc(mouseMove);
	glutMouseWheelFunc(mouseWheel);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}

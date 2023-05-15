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

#define GL_MAX_TEXTURE_MAX_ANISOTROPY     0x84FF
#define GL_TEXTURE_MAX_ANISOTROPY         0x84FE

int width = 900;
int height = 900;

glm::vec3 lightPos, lightColor, objectColor, viewPos, cameraPos;
GLuint loc1, loc2, loc3, loc4, loc5;

glm::mat4 view, proj, model;

GLuint mappingProgramID, textShaderProgram, skyboxShaderProgramID;

const GLuint i = 15;
GLuint VAO[i], VBO[i * 2], VTO[i];
GLuint mappingMode = 1;
GLuint textureMode = 7;
GLuint modelType = 'f';
char* mappingName = "Nearest Neightbour Filtering Magnification";

std::vector < ModelData > meshData;
std::vector < const char* > dataArray;
ModelData mesh_data;

bool isFirstView = false;
bool isModelRotate = false;
float camera_dis = 27.0f;

GLfloat rotate_y = 45.0f;
float init_rotate_y = 45.0f;

GLfloat prop_rotate_y = 0.0f;
bool isCameraRotate = false;
GLfloat cameraRotateY = 0.0f;

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

	loc1 = glGetAttribLocation(mappingProgramID, "vertex_position");
	loc2 = glGetAttribLocation(mappingProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(mappingProgramID, "vertex_texture");
	loc4 = glGetAttribLocation(mappingProgramID, "aTangent");
	loc5 = glGetAttribLocation(mappingProgramID, "aBitangent");

	for (int i = 0; i < dataArray.size(); i++) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, VTO[texCounter]);
		glUniform1i(glGetUniformLocation(mappingProgramID, "diffuseMap"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, VTO[texCounter + 1]);
		glUniform1i(glGetUniformLocation(mappingProgramID, "normalMap"), 1);
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

	//  Nearest Neightbour Filtering Magnification
	if (i == 0) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else if (i == 1) {  // BiLinear Filtering Magnification
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (i == 2) {  // MIP Mapped Grid
		// use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (i == 3) {  // Anisotripoc Filtering Grid
		float amount = fminf(4.0f, GL_MAX_TEXTURE_MAX_ANISOTROPY);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, amount);
	}

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

void drawModels(glm::vec3 pos) {
	// ------------ Model ------------
	glUseProgram(mappingProgramID);

	model = glm::scale(model, glm::vec3(5));
	model = glm::rotate(model, glm::radians(rotate_y), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(offsetX), glm::vec3(1, 0, 0));
	model = glm::translate(model, pos);

	glUniformMatrix4fv(glGetUniformLocation(mappingProgramID, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(mappingProgramID, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(mappingProgramID, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// Camera 
	glUniform3fv(glGetUniformLocation(mappingProgramID, "lightPos"), 1, &lightPos[0]);
	glUniform3fv(glGetUniformLocation(mappingProgramID, "viewPos"), 1, &cameraPos[0]);

	glUniform1i(glGetUniformLocation(mappingProgramID, "texture1"), 0);
	glBindVertexArray(VAO[0]);
	glActiveTexture(GL_TEXTURE0);

	if (mappingMode == 1) glBindTexture(GL_TEXTURE_2D, VTO[0]);
	if (mappingMode == 2) glBindTexture(GL_TEXTURE_2D, VTO[1]);
	if (mappingMode == 3) glBindTexture(GL_TEXTURE_2D, VTO[2]);
	if (mappingMode == 4) glBindTexture(GL_TEXTURE_2D, VTO[3]);
	
	glDrawArrays(GL_TRIANGLES, 0, meshData[0].mPointCount);
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
	view = glm::lookAt(glm::vec3(camera_dis * sin(radian), 5.0f + offsetY, 0.0f + camera_dis * cos(radian)),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
	model = glm::mat4(1.0f);

	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	lightPos = glm::vec3(0.0f, 0.0f, 2.0f);

	// ------------------------------------------------- SKYBOX ------------------------------------------------- 
	drawSkybox();

	// ------------------------------------------------- TEXT ------------------------------------------------- 
	// Show Project Title
	drawText("Filter Compare", 3, glm::vec3(5.4, 5.5, 0));

	char array[30];
	//snprintf(array, sizeof(array), "LightX: %1.1f", offsetX);
	//drawText(array, 2, glm::vec3(2.4f, -4.5f, 0.0f));
	//snprintf(array, sizeof(array), "LightY: %1.1f", offsetY);
	//drawText(array, 2, glm::vec3(5.4f, -4.5f, 0.0f));
	//snprintf(array, sizeof(array), "LightZ: %1.1f", offsetZ);
	//drawText(array, 2, glm::vec3(8.4f, -4.5f, 0.0f));

	snprintf(array, sizeof(array), "Name: %s", mappingName);
	drawText(array, 2, glm::vec3(5.4f, -4.5f, 0.0f), glm::vec4(237.0/255, 208.0/255, 2.0/255, 1));

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

		cout << rotate_y << endl;
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
	mappingProgramID = CompileShaders("./shaders/Tex_Vertex.txt", "./shaders/Tex_Fragment.txt");
	textShaderProgram = CompileShaders("./shaders/textVertexShader.txt", "./shaders/textFragmentShader.txt");
	skyboxShaderProgramID = CompileShaders("./shaders/skyboxVertexShader.txt", "./shaders/skyboxFragmentShader.txt");

	// Skybox
	generateSkybox();
	cubemapTexture = loadCubemap(faces);

	// Textures
	// Nearest Neightbour Filtering Magnification
	unsigned int Nearest = loadTexture("./Textures/grid1.jpg", 0);
	// BiLinear Filtering Magnification
	unsigned int BiLinear = loadTexture("./Textures/grid1.jpg", 1);
	// MIP Mapped Grid 
	unsigned int MIP = loadTexture("./Textures/grid1.jpg", 2);
	// Anisotropic  Filtering Grid
	unsigned int Anisotripoc = loadTexture("./Textures/grid1.jpg", 3);


	// load mesh into a vertex buffer array
	dataArray.push_back("./models/plane.dae");

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
			rotate_y = init_rotate_y;
			cameraRotateY = 0.0f;
			camera_dis = 27.0f;
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
			mappingName = "Nearest Neightbour";
			break;
		case('2'):
			mappingMode = 2;
			mappingName = "BiLinear Filtering";
			break; 
		case('3'):
			mappingMode = 3;
			mappingName = "MIP Mapped Filtering";
			break;
		case('4'):
			mappingMode = 4;
			mappingName = "Anisotropic Filtering";
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
		rotate_y -= 2;
		break;
	case(GLUT_KEY_RIGHT):
		rotate_y += 2;
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
	cout << camera_dis << endl;
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

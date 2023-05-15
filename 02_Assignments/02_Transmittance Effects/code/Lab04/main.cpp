// This code originally was the starter code provided for the CS7GV6 - Computer Graphics module project.
// I have used this code and modified it for the purposes of CS7GV3 - Real-Time Rendering Lab 2.
// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> 
#include <map>

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// GLM includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Assimp includes
#include <assimp/cimport.h> 
#include <assimp/scene.h> 
#include <assimp/postprocess.h> 

// Loading photos
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define GLT_IMPLEMENTATION
#include "gltext.h"
// Project includes
#include "maths_funcs.h"

// Include FreeType
#include <ft2build.h>
#include FT_FREETYPE_H 

/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
#define SQUARE_MESH "./models/square.dae"
#define SNOWMAN_MESH "./models/snowman.dae"
#define TEAPOT_MESH "./models/teapot.dae"
#define BOMB_MESH "./models/Cargo.dae"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

// Camera

glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 startingCameraPosition = glm::vec3(0.0f, 0.0f, 12.0f);
glm::vec3 startingCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 startingCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

#pragma region SimpleTypes
typedef struct
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
} ModelData;
#pragma endregion SimpleTypes

using namespace std;
GLuint reflectionShaderProgramID, refractionShaderProgramID, fresnelShaderProgramID, chromaticDispersionShaderProgramID, skyboxShaderProgramID, textShaderProgramID;

glm::mat4 view, proj, model;

ModelData mesh_data;
int width = 1000;
int height = 1000;

float delta;
GLuint loc1, loc2;
GLfloat rotate_y = 0.0f;

GLfloat camera_rotate_y = 0.0f;
GLfloat camera_dis = 15.0f;
bool isCameraRotate = false;
bool isModelRotate = false;

bool isSkyBlue = true;

const GLuint i = 6;
GLuint VAO[i], VBO[i * 2];
std::vector < ModelData > meshData;
std::vector < const char* > dataArray;

float eta = 0.5;
float etaR = 0.65;
float etaG = 0.67;
float etaB = 0.69;
float fPower;

float yaw = 0, pitch = 0;
float xpos = 0, ypos = 0;

int displayMode = 0;

// ------------ SKYBOX ------------
unsigned int skyboxVAO, skyboxVBO;
unsigned int cubemapTexture;
vector<std::string> faces
{
	"./skybox/skybox/right.jpg",
	"./skybox/skybox/left.jpg",
	"./skybox/skybox/top.jpg",
	"./skybox/skybox/bottom.jpg",
	"./skybox/skybox/front.jpg",
	"./skybox/skybox/back.jpg",
};
vector<std::string> faces4
{
	"./skybox/skybox4/right.bmp",
	"./skybox/skybox4/left.bmp",
	"./skybox/skybox4/top.bmp",
	"./skybox/skybox4/bottom.bmp",
	"./skybox/skybox4/front.bmp",
	"./skybox/skybox4/back.bmp",
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
		aiProcess_Triangulate | aiProcess_PreTransformVertices
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
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING

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

GLuint CompileShaders(const char* vertexShader, const char* fragmentShader)
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
	AddShader(shaderProgramID, vertexShader, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, fragmentShader, GL_FRAGMENT_SHADER);

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
// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh(std::vector < const char* > dataArray) {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	loc1 = glGetAttribLocation(reflectionShaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(reflectionShaderProgramID, "vertex_normal");
	//loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");
	int counter = 0;
	for (int i = 0; i < dataArray.size(); i++) {
		mesh_data = load_mesh(dataArray[i]);
		meshData.push_back(mesh_data);

		glGenBuffers(1, &VBO[counter]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[counter]);
		glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &VBO[counter + 1]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[counter + 1]);
		glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);

		glGenVertexArrays(1, &VAO[i]);
		glBindVertexArray(VAO[i]);

		glEnableVertexAttribArray(loc1);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[counter]);
		glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glEnableVertexAttribArray(loc2);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[counter + 1]);
		glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		counter += 2;
	}
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
#pragma endregion VBO_FUNCTIONS

void drawText(const char* str, GLfloat size, glm::vec3 pos) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
	glUseProgram(textShaderProgramID);
	glUniformMatrix4fv(glGetUniformLocation(textShaderProgramID, "proj"), 1, GL_FALSE, glm::value_ptr(projection));

	// Initialize glText
	gltInit();

	// Creating text
	GLTtext* text = gltCreateText();
	gltSetText(text, str);

	// Begin text drawing (this for instance calls glUseProgram)
	gltBeginDraw();

	// Draw any amount of text between begin and end
	gltColor(1.0f, 1.0f, 1.0f, 1.0f);
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

void drawModelByType(GLuint shaderProgramID, GLuint VAO, ModelData meshData, glm::vec3 pos, float scale = 1.0, const char* type = "", float paramVal = 0.5) {
	// Use the specified shader program and vertex array object
	glUseProgram(shaderProgramID);
	glBindVertexArray(VAO);

	// Set the view and projection matrices as uniform variables in the shader program
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

	// Create a model matrix and apply transformations for position, rotation, and scale
	model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	model = glm::rotate(model, glm::radians(rotate_y), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f * scale, 1.0f * scale, 1.0f * scale));

	// Set additional uniform variables depending on the value of "type"
	if (type == "fresnel") {
		glUniform1f(glGetUniformLocation(fresnelShaderProgramID, "eta"), paramVal);
	}
	else if (type == "Chromatic") {
		// Set the refractive indices for the three color channels
		etaR = 0.57;
		etaG = 0.59;
		etaB = 0.61;
		glUniform1f(glGetUniformLocation(chromaticDispersionShaderProgramID, "etaR"), etaR);
		glUniform1f(glGetUniformLocation(chromaticDispersionShaderProgramID, "etaG"), etaG);
		glUniform1f(glGetUniformLocation(chromaticDispersionShaderProgramID, "etaB"), etaB);
		// Set the power value for the chromatic dispersion effect
		glUniform1f(glGetUniformLocation(chromaticDispersionShaderProgramID, "fPower"), paramVal);
	}

	// Set the model matrix as a uniform variable in the shader program and draw the mesh
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glDrawArrays(GL_TRIANGLES, 0, meshData.mPointCount);
}

void showModelByType(int type) {
	switch (type)
	{
	case 0:
		// model 1 => Reflection
		drawModelByType(reflectionShaderProgramID, VAO[0], meshData[0], glm::vec3(-2.0f, 1.0f, 0.0f));
		drawText("Reflection", 2, glm::vec3(3.0f, -1.0f, 0.0f));

		// model 2 => Refraction
		drawModelByType(refractionShaderProgramID, VAO[0], meshData[0], glm::vec3(-2.0f, -2.0f, 0.0f));
		drawText("Refraction", 2, glm::vec3(9.0f, -1.0f, 0.0f));

		// model 3 => Fresnel
		drawModelByType(refractionShaderProgramID, VAO[0], meshData[0], glm::vec3(2.0f, 1.0f, 0.0f), 1.0, "fresnel", 0.2);
		drawText("Fresnel", 2, glm::vec3(3.0f, -6.0f, 0.0f));

		// model 4 => Chromatic Dispersion
		drawModelByType(chromaticDispersionShaderProgramID, VAO[0], meshData[0], glm::vec3(2.0f, -2.0f, 0.0f), 1.0, "Chromatic", 2.0);
		drawText("Chromatic Dispersion", 2, glm::vec3(9.0f, -6.0f, 0.0f));
		break;
	case 1:   // Reflection
		drawText("Reflection", 3, glm::vec3(6.4f, -4.f, 0.0f));
		// model 1 => snowman
		drawModelByType(reflectionShaderProgramID, VAO[1], meshData[1], glm::vec3(-4.0f, 0.0f, 0.0f), 0.4);

		// model 2 => teapot
		drawModelByType(reflectionShaderProgramID, VAO[2], meshData[2], glm::vec3(0.0f, 0.0f, 0.0f));

		// model 3 => bomb
		drawModelByType(reflectionShaderProgramID, VAO[3], meshData[3], glm::vec3(4.0f, -1.0f, 0.0f), 0.5);
		break;
	case 2:   // Refraction
		drawText("Refraction", 3, glm::vec3(6.4f, -4.f, 0.0f));
		// model 1 => snowman
		drawModelByType(refractionShaderProgramID, VAO[1], meshData[1], glm::vec3(-4.0f, 0.0f, 0.0f), 0.4);

		// model 2 => teapot
		drawModelByType(refractionShaderProgramID, VAO[2], meshData[2], glm::vec3(0.0f, 0.0f, 0.0f));

		// model 3 => bomb
		drawModelByType(refractionShaderProgramID, VAO[3], meshData[3], glm::vec3(4.0f, -1.0f, 0.0f), 0.5);
		break;
	case 3:   // Fresnel
		drawText("Fresnel component", 3, glm::vec3(6.4f, -4.f, 0.0f));
		// model 1 => teapot => eta: 0.1
		drawModelByType(fresnelShaderProgramID, VAO[2], meshData[2], glm::vec3(-4.0f, 0.0f, 0.0f), .8, "fresnel", 0.1);
		drawText("eta: 0.1", 2, glm::vec3(1.4f, -3.f, 0.0f));

		// model 2 => teapot => eta: 0.5
		drawModelByType(fresnelShaderProgramID, VAO[2], meshData[2], glm::vec3(0.0f, 0.0f, 0.0f), .8, "fresnel", 0.5);
		drawText("eta: 0.5", 2, glm::vec3(6.4f, -3.f, 0.0f));

		// model 3 => teapot => eta: 0.9
		drawModelByType(fresnelShaderProgramID, VAO[2], meshData[2], glm::vec3(4.0f, 0.0f, 0.0f), .8, "fresnel", 0.9);
		drawText("eta: 0.9", 2, glm::vec3(11.4f, -3.f, 0.0f));
		break;
	case 4:  // Chromatic Dispersion
		drawText("Chromatic Dispersion", 3, glm::vec3(6.4f, -4.f, 0.0f));
		// model 1 => teapot => fPower: 0.5
		drawText("fPower: 0.5", 2, glm::vec3(1.4f, -3.f, 0.0f));
		drawModelByType(chromaticDispersionShaderProgramID, VAO[2], meshData[2], glm::vec3(-4.0f, 0.0f, 0.0f), .8, "Chromatic", .5);

		// model 2 => teapot => fPower: 1.0
		drawText("fPower: 1.0", 2, glm::vec3(6.4f, -3.f, 0.0f));
		drawModelByType(chromaticDispersionShaderProgramID, VAO[2], meshData[2], glm::vec3(0.0f, 0.0f, 0.0f), .8, "Chromatic", 1.0);

		// model 3 => teapot => fPower: 2.0
		drawText("fPower: 2.0", 2, glm::vec3(11.4f, -3.f, 0.0f));
		drawModelByType(chromaticDispersionShaderProgramID, VAO[2], meshData[2], glm::vec3(4.0f, 0.0f, 0.0f), .8, "Chromatic", 2.0);
		break;
	default:
		break;
	}
}

void display() {

	// Enable depth test and specify depth comparison function
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Set clear color and clear buffers
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Define the view matrix
	view = glm::mat4(1.0f); // Initialize view matrix to identity matrix
	float radian = glm::radians(camera_rotate_y); // Convert camera rotation angle to radians
	view = glm::lookAt(
		glm::vec3(camera_dis * sin(radian), 0.0f, 0.0f + camera_dis * cos(radian)), // Camera position
		glm::vec3(0.0f, 0.0f, 0.0f), // Target position
		glm::vec3(0.0f, 1.0f, 0.0f) // Up vector
	);

	// Use starting camera position, front, and up if displayMode is false
	if (!displayMode) {
		view = glm::lookAt(startingCameraPosition, startingCameraPosition + startingCameraFront, startingCameraUp);
	}

	// Define the projection matrix
	proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);

	// Initialize the model matrix to identity matrix
	model = glm::mat4(1.0f);

	// Draw skybox
	drawSkybox();

	// Draw model
	drawText("Transmittance Effects", 3, glm::vec3(6, 5, 0)); // Draw text at specified position
	showModelByType(displayMode);

	// Swap the front and back buffers to display the rendered image
	glutSwapBuffers();
}

void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	// Rotate the model slowly around the y axis at 20 degrees per second

	rotate_y += 25.0f * delta;
	rotate_y = fmodf(rotate_y, 360.0f);


	if (isCameraRotate) {
		camera_rotate_y += 30.0f * delta;
		camera_rotate_y = fmodf(camera_rotate_y, 360.0f);
	}

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Set up the shaders
	reflectionShaderProgramID = CompileShaders("./shaders/modelVertexShader.txt", "./shaders/reflectionFragmentShader.txt");
	refractionShaderProgramID = CompileShaders("./shaders/modelVertexShader.txt", "./shaders/refractionFragmentShader.txt");
	fresnelShaderProgramID = CompileShaders("./shaders/modelVertexShader.txt", "./shaders/fresnelFragmentShader.txt");
	chromaticDispersionShaderProgramID = CompileShaders("./shaders/modelVertexShader.txt", "./shaders/chromaticDispersionFragmentShader.txt");
	skyboxShaderProgramID = CompileShaders("./shaders/skyboxVertexShader.txt", "./shaders/skyboxFragmentShader.txt");
	textShaderProgramID = CompileShaders("./shaders/textVertexShader.txt", "./shaders/textFragmentShader.txt");
	// Skybox
	generateSkybox();
	cubemapTexture = loadCubemap(faces);
	// load mesh into a vertex buffer array
	dataArray.push_back(SQUARE_MESH);
	dataArray.push_back(SNOWMAN_MESH);
	dataArray.push_back(TEAPOT_MESH);
	dataArray.push_back(BOMB_MESH);
	generateObjectBufferMesh(dataArray);

}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
	switch (key) {
	case '1':
		displayMode = 0;
		break;
	case '2':
		displayMode = 1;
		break;
	case '3':
		displayMode = 2;
		break;
	case '4':
		displayMode = 3;
		break;
	case '5':
		displayMode = 4;
		break;
	case 'c':
		isCameraRotate = !isCameraRotate;
		break;
	case 'm':
		isModelRotate = !isModelRotate;
		break;
	case 'r':
		camera_rotate_y = 0.0;
		camera_dis = 16.0;
		break;
	case 'p':
		isSkyBlue = !isSkyBlue;
		cubemapTexture = loadCubemap(isSkyBlue ? faces : faces4);
		break;

	}
}

void specialKeypress(int key, int x, int y) {
	switch (key) {
		// Specular Lighting settings, up for up, down for down
	case(GLUT_KEY_UP):
		camera_dis += 0.2f;
		break;
	case(GLUT_KEY_DOWN):
		camera_dis -= 0.2f;
		break;
	}
};

void mouseWheel(int key, int wheeldir, int x, int y) {
	if (wheeldir == 1)
	{
		camera_dis += 0.1f;
	}
	else if (wheeldir == -1) {
		camera_dis -= 0.1f;
	}
}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Lab 2 Transmittance Effects");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);
	glutSpecialFunc(specialKeypress);
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

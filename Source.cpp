#include <iostream>
#include <cstdlib>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Camera.h"
#include "cylinder.h"
using namespace std;

#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

namespace {
	// Window Title
	const char* const WINDOW_TITLE = "Devyn Mustard's FINAL 3D Scene";

	// Variables for window width and height
	const int WINDOW_WIDTH = 1920;
	const int WINDOW_HEIGHT = 1080;


	// Base camera settings
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
	//glm::vec4 cameraRotation = glm::rotate(15, cameraPos, cameraFront);
	// Time between frames
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	// Lamp variables
	glm::vec3 spotLightPosition(18.0f, 0.0f, 4.0f);
	glm::vec3 spotLightScale(0.4f);
	glm::vec3 spotLightColor(0.99f, 0.7f, 0.4f);

	glm::vec3 testColor(1.0f, 1.0f, 0.0f);
	glm::vec3 testColorPosition(2.0f, -2.5f, -2.5f);
	
	glm::vec3 objectColors(1.0f, 1.0f, 1.0f);

	glm::vec3 keyLightPosition(0.0f, 20.0f, 0.0f);
	glm::vec3 keyLightScale(0.45f);
	glm::vec3 keyLightColor(0.95f, 0.95f, 1.0f);

	// Structure of pyramid Obj
	struct PyramidObj {
		GLuint vao[10];
		GLuint vbo[10];
		GLuint nvertices[10];
	};

}

// Init Variables
GLFWwindow* window = nullptr;
PyramidObj pyramidMesh;

GLuint programId;
GLuint lampProgramId;
GLuint phoneTex, desktopTex, screenTex, pencilBaseTex, eraserTex, keyboardTex, laptopScreenTex, trackpadTex, lampTex;
glm::vec2 gUVScale(1.0f, 1.0f);
GLint gTexWrapMode = GL_REPEAT;

// Camera INIT
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastXPos = WINDOW_WIDTH / 2.0f;
float lastYPos = WINDOW_HEIGHT / 2.0f;
bool mouse = true;
glm::mat4 projection = glm::perspective(glm::radians(ZOOM), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);


// Predefined Function Declerations
bool InitializeWindow(int, char* [], GLFWwindow** window);
void ResizeWindow(GLFWwindow* window, int width, int height);
void PollInput(GLFWwindow* window);
void drawPyramid(PyramidObj& pyramidMesh);
void erasePyramid(PyramidObj& pyramidMesh);
void renderFrame();
void createShader(const char* vertexShader, const char* fragmentShader, GLuint& programId);
void eraseShader(GLuint programId);
bool renderTexture(const char* filename, GLuint& textureId);
void eraseTexture(GLuint textureId);
void MousePositionCall(GLFWwindow* window, double xLoc, double yLoc);
void MouseScrollPositionCall(GLFWwindow* window, double xOffset, double yOffset);
void MouseButtonCall(GLFWwindow* window, int button, int action, int modifier);


// Shape Vertex Shader Source Code
const GLchar* vertexShader = GLSL(440,
	layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f);
	vertexFragmentPos = vec3(model * vec4(position, 1.0f));
	vertexNormal = mat3(transpose(inverse(model))) * normal;
	vertexTextureCoordinate = textureCoordinate;
}
);

// Shape Fragment Shader Source Code
const GLchar* fragmentShader = GLSL(440,

	in vec3 vertexFragmentPos;
in vec3 vertexNormal;
in vec2 vertexTextureCoordinate; // for texture coordinates, not color

out vec4 fragmentColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 keyLightColor;
uniform vec3 lightPos;
uniform vec3 keyLightPos;
uniform vec3 viewPosition;

uniform sampler2D uTexture;
uniform vec2 uvScale;

void main()
{
	//Calculate Ambient lighting*/
	float spotStrength = 0.1f; // Set ambient or global lighting strength
	float keyStrength = 1.0f; // Set ambient or global lighting strength
	vec3 spot = spotStrength * lightColor; // Generate ambient light color
	vec3 key = keyStrength * keyLightColor;

	//Calculate Diffuse lighting*/
	vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
	vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	vec3 keyLightDirection = normalize(keyLightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube

	float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	float keyImpact = max(dot(norm, keyLightDirection), 1.0);// Calculate diffuse impact by generating dot product of normal and light

	vec3 diffuse = impact * lightColor; // Generate diffuse light color
	vec3 keyDiffuse = keyImpact * keyLightColor;

	//Calculate Specular lighting*/
	float specularIntensity = 0.4f; // Set specular light strength
	float highlightSize = 16.0f; // Set specular highlight size
	vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
	vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
	//Calculate specular component
	float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
	vec3 specular = specularIntensity * specularComponent * lightColor;
	vec3 keySpecular = specularIntensity * specularComponent * keyLightColor;

	// Texture holds the color to be used for all three components
	vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

	// Calculate phong result
	vec3 phong = (spot + key + diffuse /*+ keyDiffuse*/ + specular /*+ objectColor*/) * textureColor.xyz;

	fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU

}
);






// Light Shader Source Code
const GLchar* lampVertexShaderSource = GLSL(440,

	layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);

// Light Fragment Shader Source Code
const GLchar* lampFragmentShaderSource = GLSL(440,

	out vec4 fragmentColor; // For outgoing light color to the GPU

void main()
{
	fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);

// main function, calls on functions, renders, and 
// checks for errors 
int main(int argc, char* argv[]) {
	if (!InitializeWindow(argc, argv, &window))
		return EXIT_FAILURE;

	// Creating Pyramid Objects
	drawPyramid(pyramidMesh);


	// Creating Both Vertex and Fragment Shaders
	createShader(vertexShader, fragmentShader, programId);
	createShader(lampVertexShaderSource, lampFragmentShaderSource, lampProgramId);
	// Black Window Color
	glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

	// Load Texture
	const char* phoneTexFile = "resources/textures/phonebase.png";
	const char* desktopTexFile = "resources/textures/desktop.png";
	const char* screenTexFile = "resources/textures/screen.png";
	const char* pencilBaseTexFile = "resources/textures/pencilBody.png";
	const char* eraserTexFile = "resources/textures/eraser.png";
	const char* keyboardTexFile = "resources/textures/keyboard.png";
	const char* laptopScreenTexFile = "resources/textures/laptopScreen.png";
	const char* trackpadTexFile = "resources/textures/trackpad.png";
	const char* lampTexFile = "resources/textures/lamp.png";
	if (!renderTexture(phoneTexFile, phoneTex))
	{
		cout << "Failed to load texture " << phoneTex << endl;
		return EXIT_FAILURE;
	}
	if (!renderTexture(desktopTexFile, desktopTex))
	{
		cout << "Failed to load texture " << desktopTex << endl;
		return EXIT_FAILURE;
	}
	if (!renderTexture(screenTexFile, screenTex))
	{
		cout << "Failed to load texture " << screenTex << endl;
		return EXIT_FAILURE;
	}
	if (!renderTexture(pencilBaseTexFile, pencilBaseTex))
	{
		cout << "Failed to load texture " << pencilBaseTex << endl;
		return EXIT_FAILURE;
	}
	if (!renderTexture(eraserTexFile, eraserTex))
	{
		cout << "Failed to load texture " << eraserTex << endl;
		return EXIT_FAILURE;
	}
	if (!renderTexture(keyboardTexFile, keyboardTex))
	{
		cout << "Failed to load texture " << keyboardTex << endl;
		return EXIT_FAILURE;
	}
	if (!renderTexture(laptopScreenTexFile, laptopScreenTex))
	{
		cout << "Failed to load texture " << laptopScreenTex << endl;
		return EXIT_FAILURE;
	}
	if (!renderTexture(trackpadTexFile, trackpadTex))
	{
		cout << "Failed to load texture " << trackpadTex << endl;
		return EXIT_FAILURE;
	}
	if (!renderTexture(lampTexFile, lampTex))
	{
		cout << "Failed to load texture " << lampTex << endl;
		return EXIT_FAILURE;
	}

	glUseProgram(programId);
	glUniform1i(glGetUniformLocation(programId, "mainTexture"), 0);

	// while loop that calls render functions and polls events
	while (!glfwWindowShouldClose(window)) {
		PollInput(window);

		// Call to render frame function
		renderFrame();

		// Poll events every loop
		glfwPollEvents();
	}

	// Erase mesh data of pyramid
	erasePyramid(pyramidMesh);


	// Erase shader data
	eraseShader(programId);

	// Erase Texture Data
	eraseTexture(phoneTex);
	eraseTexture(desktopTex);
	eraseTexture(screenTex);
	eraseTexture(pencilBaseTex);
	eraseTexture(eraserTex);
	// Close program
	exit(EXIT_SUCCESS);
}


// Begin GLFW Init, and create window
bool InitializeWindow(int argc, char* argv[], GLFWwindow** window) {
	// GLFW Init
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// GLFW Window Creation
	*window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		cout << "Failed to create window" << endl;
		glfwTerminate();
		return false;
	}

	// Mouse Input GLFW call back functions
	glfwSetCursorPosCallback(*window, MousePositionCall);
	glfwSetScrollCallback(*window, MouseScrollPositionCall);
	glfwSetMouseButtonCallback(*window, MouseButtonCall);

	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Resize frame function
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, ResizeWindow);

	// GLEW Init
	glewExperimental = GL_TRUE;
	GLenum GLEWInit = glewInit();

	if (GLEW_OK != GLEWInit) {
		std::cerr << glewGetErrorString(GLEWInit) << std::endl;
		return false;
	}
	return true;
}

// Rotate every texture loaded
void flipImage(unsigned char* image, int width, int height, int channels)
{
	for (int j = 0; j < height / 2; ++j)
	{
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;

		for (int i = width * channels; i > 0; --i)
		{
			unsigned char tmp = image[index1];
			image[index1] = image[index2];
			image[index2] = tmp;
			++index1;
			++index2;
		}
	}
}


// Draw Trianlge Function, providing the vertex data and adding it to buffer
void drawPyramid(PyramidObj& pyramidMesh) {
	
	// Count of vertex floats
	const GLuint vertexFloats = 3;
	// Count of floats per tex
	const GLuint textureFloats = 2;
	
	const GLuint normalFloats = 3;
	// Adding each float value between each new vertex 
	GLint stride = sizeof(float) * (vertexFloats + normalFloats + textureFloats);

	GLfloat cellphoneVerts[] = {

		 1.0f, -0.5f, -0.5f, 0.0, 0.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -0.5f, -0.5f, 0.0, 0.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  0.5f, -0.5f, 0.0, 0.0f, -1.0f, 1.0f, 1.0f,
		 1.0f,  0.5f, -0.5f, 0.0, 0.0f, -1.0f, 1.0f, 1.0f,
		-1.0f,  0.5f, -0.5f, 0.0, 0.0f, -1.0f, 0.0f, 1.0f,
		-1.0f, -0.5f, -0.5f, 0.0, 0.0f, -1.0f, 0.0f, 0.0f,
									   
		-1.0f, -0.5f, -0.4f, 0.0, 0.0f, 1.0f, 0.0f, 0.0f,
		 1.0f, -0.5f, -0.4f, 0.0, 0.0f, 1.0f, 1.0f, 0.0f,
		 1.0f,  0.5f, -0.4f, 0.0, 0.0f, 1.0f, 1.0f, 1.0f,
		 1.0f,  0.5f, -0.4f, 0.0, 0.0f, 1.0f, 1.0f, 1.0f,
		-1.0f,  0.5f, -0.4f, 0.0, 0.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, -0.5f, -0.4f, 0.0, 0.0f, 1.0f, 0.0f, 0.0f,

		-1.0f,  0.5f, -0.4f, 0.0, -1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f,  0.5f, -0.5f, 0.0, -1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, -0.5f, -0.5f, 0.0, -1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -0.5f, -0.5f, 0.0, -1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -0.5f, -0.4f, 0.0, -1.0f, 0.0f, 0.0f, 0.0f,
		-1.0f,  0.5f, -0.4f, 0.0, -1.0f, 0.0f, 1.0f, 0.0f,

		 1.0f,  0.5f, -0.4f, 0.0, 1.0f, 0.0f, 1.0f, 0.0f,
		 1.0f,  0.5f, -0.5f, 0.0, 1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -0.5f, -0.5f, 0.0, 1.0f, 0.0f, 0.0f, 1.0f,
		 1.0f, -0.5f, -0.5f, 0.0, 1.0f, 0.0f, 0.0f, 1.0f,
		 1.0f, -0.5f, -0.4f, 0.0, 1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  0.5f, -0.4f, 0.0, 1.0f, 0.0f, 1.0f, 0.0f,

		-1.0f, -0.5f, -0.5f, -1.0, 0.0f, 0.0f, 0.0f, 1.0f,
		 1.0f, -0.5f, -0.5f, -1.0, 0.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -0.5f, -0.4f, -1.0, 0.0f, 0.0f, 1.0f, 0.0f,
		 1.0f, -0.5f, -0.4f, -1.0, 0.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, -0.5f, -0.4f, -1.0, 0.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, -0.5f, -0.5f, -1.0, 0.0f, 0.0f, 0.0f, 1.0f,
							 
		-1.0f,  0.5f, -0.5f, 1.0, 0.0f, 0.0f, 0.0f, 1.0f,
		 1.0f,  0.5f, -0.5f, 1.0, 0.0f, 0.0f, 1.0f, 1.0f,
		 1.0f,  0.5f, -0.4f, 1.0, 0.0f, 0.0f, 1.0f, 0.0f,
		 1.0f,  0.5f, -0.4f, 1.0, 0.0f, 0.0f, 1.0f, 0.0f,
		-1.0f,  0.5f, -0.4f, 1.0, 0.0f, 0.0f, 0.0f, 0.0f,
		-1.0f,  0.5f, -0.5f, 1.0, 0.0f, 0.0f, 0.0f, 1.0f,
	};

	GLfloat deskVerts[] = {
		10.0f, -3.87f, -0.51f,0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-3.0f, 1.0f, -0.51f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-3.0f, -3.87, -0.51,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		10.0f, -3.87f, -0.51f,0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		10.0f, 1.0f, -0.51f,  0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		-3.0f, 1.0f, -0.51f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f
	};

	GLfloat phonescreenVerts[] = {
		0.85f, -0.45f, -0.399f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-0.85f, 0.45f, -0.399f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		-0.85f, -0.45f, -0.399f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
		0.85f, -0.45f, -0.399f, 0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
		0.85f, 0.45f, -0.399f,  0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		-0.85f, 0.45f, -0.399f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	};

	GLfloat laptopBaseVerts[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};

	GLfloat pyramidLightVerts[] = {
				  -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
				  0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
				  0.0f, 0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 1.0f,

				  -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
				  0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
				  0.0f, 0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 0.5f, 1.0f,

				  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				  -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f, 0.0f, 1.0f, 0.5f,
				  0.0f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.5f, 1.0f,

				  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
				  0.5f, -0.5f, 0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				  0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f,

				  0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
				  0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
				  0.0f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 1.0f,

				  -0.5F, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
				  -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
				  0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5, 1.0f

	};

	// Set nvertices variable with size of objects verticies
	pyramidMesh.nvertices[0] = sizeof(cellphoneVerts) / sizeof(cellphoneVerts[0]) * (vertexFloats + normalFloats + textureFloats);
	pyramidMesh.nvertices[1] = sizeof(deskVerts) / (sizeof(deskVerts[0]) * (vertexFloats + normalFloats + textureFloats));
	pyramidMesh.nvertices[2] = sizeof(phonescreenVerts) / (sizeof(phonescreenVerts[0]) * (vertexFloats + normalFloats + textureFloats));
	pyramidMesh.nvertices[4] = sizeof(laptopBaseVerts) / (sizeof(laptopBaseVerts[0]) * (vertexFloats + normalFloats + textureFloats));
	pyramidMesh.nvertices[5] = sizeof(pyramidLightVerts) / (sizeof(pyramidLightVerts[0]) * (vertexFloats + normalFloats + textureFloats));

// Generate Cellphone object
	glGenVertexArrays(1, &pyramidMesh.vao[0]);
	glGenBuffers(1, &pyramidMesh.vbo[0]);
	glBindVertexArray(pyramidMesh.vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, pyramidMesh.vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cellphoneVerts), cellphoneVerts, GL_STATIC_DRAW);
	
	// Position Attrib
	glVertexAttribPointer(0, vertexFloats, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	// Normal Attrib
	glVertexAttribPointer(1, normalFloats, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * vertexFloats));
	glEnableVertexAttribArray(1);

	// Texture Attrib
	glVertexAttribPointer(2, textureFloats, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (vertexFloats + normalFloats)));
	glEnableVertexAttribArray(2);


// Generate Desk object
	glGenVertexArrays(1, &pyramidMesh.vao[1]);
	glGenBuffers(1, &pyramidMesh.vbo[1]);
	glBindVertexArray(pyramidMesh.vao[1]);
	glBindBuffer(GL_ARRAY_BUFFER, pyramidMesh.vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(deskVerts), deskVerts, GL_STATIC_DRAW);

	// Position Attrib
	glVertexAttribPointer(0, vertexFloats, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	// Normal Attrib
	glVertexAttribPointer(1, normalFloats, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* vertexFloats));
	glEnableVertexAttribArray(1);

	// Texture Attrib
	glVertexAttribPointer(2, textureFloats, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (vertexFloats + normalFloats)));
	glEnableVertexAttribArray(2);

// Generate Cell Phone Screen object
	glGenVertexArrays(1, &pyramidMesh.vao[2]);
	glGenBuffers(1, &pyramidMesh.vbo[2]);
	glBindVertexArray(pyramidMesh.vao[2]);
	glBindBuffer(GL_ARRAY_BUFFER, pyramidMesh.vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(phonescreenVerts), phonescreenVerts, GL_STATIC_DRAW);

	// Position Attrib
	glVertexAttribPointer(0, vertexFloats, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	// Normal Attrib
	glVertexAttribPointer(1, normalFloats, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* vertexFloats));
	glEnableVertexAttribArray(1);

	// Texture Attrib
	glVertexAttribPointer(2, textureFloats, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (vertexFloats + normalFloats)));
	glEnableVertexAttribArray(2);

// Generate Laptop base object
	glGenVertexArrays(1, &pyramidMesh.vao[4]);
	glGenBuffers(1, &pyramidMesh.vbo[4]);
	glBindVertexArray(pyramidMesh.vao[4]);
	glBindBuffer(GL_ARRAY_BUFFER, pyramidMesh.vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(laptopBaseVerts), laptopBaseVerts, GL_STATIC_DRAW);

	// Position Attrib
	glVertexAttribPointer(0, vertexFloats, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	// Normal Attrib
	glVertexAttribPointer(1, normalFloats, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* vertexFloats));
	glEnableVertexAttribArray(1);

	// Texture Attrib
	glVertexAttribPointer(2, textureFloats, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (vertexFloats + normalFloats)));
	glEnableVertexAttribArray(2);

// Generate Pyramid Lamp object
	glGenVertexArrays(1, &pyramidMesh.vao[5]);
	glGenBuffers(1, &pyramidMesh.vbo[5]);
	glBindVertexArray(pyramidMesh.vao[5]);
	glBindBuffer(GL_ARRAY_BUFFER, pyramidMesh.vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidLightVerts), pyramidLightVerts, GL_STATIC_DRAW);

	// Position Attrib
	glVertexAttribPointer(0, vertexFloats, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	// Normal Attrib
	glVertexAttribPointer(1, normalFloats, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* vertexFloats));
	glEnableVertexAttribArray(1);

	// Texture Attrib
	glVertexAttribPointer(2, textureFloats, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (vertexFloats + normalFloats)));
	glEnableVertexAttribArray(2);

}

// Close window when the escape key is pressed
void PollInput(GLFWwindow* window) {
	// How fast the camera will move
	static const float cameraSpeed = 2.5f;
	float cameraOffset = cameraSpeed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	// Move Camera Forward
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	// Move Camera Backwards
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);

	// Move Camera Up
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	// Move Camera Down
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);

	// Move Camera Right
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	// Move Camera Left
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);

	// Change perspective between orthographic and perspective
	static bool PerspectiveView = true;
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		cout << PerspectiveView << PerspectiveView << PerspectiveView;
		if (PerspectiveView == true) {

			projection = glm::perspective(0.12f, 800.0f / 600.0f, 0.1f, 100.0f);
			PerspectiveView = false;
		}
		else {
			projection = glm::perspective(ZOOM, 800.0f / 600.0f, 0.1f, 100.0f);
			PerspectiveView = true;
		}
	}
}

// Function to recenter viewport on window resize
void ResizeWindow(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

// Erase Objects
void erasePyramid(PyramidObj& pyramidMesh) {
	glDeleteVertexArrays(1, &pyramidMesh.vao[0]);
	glDeleteBuffers(1, &pyramidMesh.vbo[0]);
	glDeleteVertexArrays(1, &pyramidMesh.vao[1]);
	glDeleteBuffers(1, &pyramidMesh.vbo[1]);
	glDeleteVertexArrays(1, &pyramidMesh.vao[2]);
	glDeleteBuffers(1, &pyramidMesh.vbo[2]);
	glDeleteVertexArrays(1, &pyramidMesh.vao[4]);
	glDeleteBuffers(1, &pyramidMesh.vbo[4]);
	glDeleteVertexArrays(1, &pyramidMesh.vao[5]);
	glDeleteBuffers(1, &pyramidMesh.vbo[5]);
}

// Create and load Texture data
bool renderTexture(const char* filename, GLuint& textureId) {
	int widthPx, heightPx, channel;
	unsigned char* tex = stbi_load(filename, &widthPx, &heightPx, &channel, 0);
	if (tex) {
		flipImage(tex, widthPx, heightPx, channel);
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channel == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, widthPx, heightPx, 0, GL_RGB, GL_UNSIGNED_BYTE, tex);
		}
		else if (channel == 4) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, widthPx, heightPx, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		// Remove Texture
		stbi_image_free(tex);
		glBindTexture(GL_TEXTURE_2D, 0);
		// Success
		return true;
	}
	// Error
	return false;
}

void eraseTexture(GLuint textureID) {
	glGenTextures(1, &textureID);
}


// Erase shader
void eraseShader(GLuint programId) {
	glDeleteProgram(programId);
}

void MousePositionCall(GLFWwindow* window, double xLoc, double yLoc) {
	if (mouse) {
		lastXPos = xLoc;
		lastYPos = yLoc;
		mouse = false;
	}
	float xOffSet = xLoc - lastXPos;
	float yOffSet = lastYPos - yLoc;

	lastXPos = xLoc;
	lastYPos = yLoc;

	camera.ProcessMouseMovement(xOffSet, yOffSet);
	cout << "Mouse at " << xLoc << "," << yLoc;
}

void MouseScrollPositionCall(GLFWwindow* window, double xOffset, double yOffset) {
	camera.ProcessMouseScroll(yOffset);
}

void MouseButtonCall(GLFWwindow* window, int button, int action, int modifier) {
	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT: {
		if (action == GLFW_PRESS)
			cout << "Left mouse clicked";
		else cout << "Left mouse released";
	} break;
	case GLFW_MOUSE_BUTTON_MIDDLE: {
		if (action == GLFW_PRESS)
			cout << "Middle mouse clicked";
		else cout << "Middle mouse released";
	} break;
	case GLFW_MOUSE_BUTTON_RIGHT: {
		if (action == GLFW_PRESS)
			cout << "Right mouse clicked";
		else cout << "Right mouse released";
	} break;
	}
}

// Render frame function
void renderFrame() {

	// Time elapsed
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// Enable 3d axis
	glEnable(GL_DEPTH_TEST);

	// Black Background color
	glClearColor(0.06f, 0.06f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Double size of pyramid
	glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));

	// Lay scene flat
	glm::mat4 rotation = glm::rotate(-33.0f, glm::vec3(0.33f, 0.0f, 0.0f));

	// Originate the pyramids vectors
	glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

	// Apply Transformation every frame
	glm::mat4 model = rotation * scale * translation;

	glm::mat4 view = camera.GetViewMatrix();

	// Set the shader to be used
	glUseProgram(programId);

	// Retrieves and passes transform matrices to the Shader program
	GLint modelLoc = glGetUniformLocation(programId, "model");
	GLint viewLoc = glGetUniformLocation(programId, "view");
	GLint projLoc = glGetUniformLocation(programId, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glm::mat4 transformation(1.0f);

	// Get transform location for texture
	GLuint transformLocation = glGetUniformLocation(programId, "shaderTransform");
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transformation));
	GLint UVScaleLoc = glGetUniformLocation(programId, "uvScale");
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
	GLint objectColorLoc = glGetUniformLocation(programId, "objectColor");
	GLint lightColorLoc = glGetUniformLocation(programId, "lightColor");
	GLint keyLightColorLoc = glGetUniformLocation(programId, "keyLightColor");
	GLint keyLightPositionLoc = glGetUniformLocation(programId, "keyLightPos");
	GLint lightPositionLoc = glGetUniformLocation(programId, "lightPos");
	GLint viewPositionLoc = glGetUniformLocation(programId, "viewPosition");
// Render Phone -----------------------------------------------------------------------
	glUseProgram(programId);
	// Draws triangles that generate pyramid
	glBindVertexArray(pyramidMesh.vao[0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, phoneTex);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glDrawArrays(GL_TRIANGLES, 0, pyramidMesh.nvertices[0]);
   
// Render Desk -----------------------------------------------------------------------
	glUseProgram(programId);
	// Draws triangles that generate pyramid
	glBindVertexArray(pyramidMesh.vao[1]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, desktopTex);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glDrawArrays(GL_TRIANGLES, 0, pyramidMesh.nvertices[1]);
	
// Render Phone Screen -----------------------------------------------------------------------
	glUseProgram(programId);
	glBindVertexArray(pyramidMesh.vao[2]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screenTex);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glDrawArrays(GL_TRIANGLES, 0, pyramidMesh.nvertices[2]);
	glUseProgram(programId);


	// Render Laptop Keyboard -----------------------------------------------------------------------
	glUseProgram(programId);
	glBindVertexArray(pyramidMesh.vao[2]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, keyboardTex);
	scale = glm::scale(glm::vec3(4.5f, 4.0f, 0.12f));

	translation = glm::translate(glm::vec3(2.0f, -0.3f, -6.9f));

	// Apply Transformation every frame
	model = rotation * scale * translation;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glDrawArrays(GL_TRIANGLES, 0, pyramidMesh.nvertices[2]);
	glUseProgram(programId);


	// Render Laptop TrackPad -----------------------------------------------------------------------
	glUseProgram(programId);
	glBindVertexArray(pyramidMesh.vao[2]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, trackpadTex);
	scale = glm::scale(glm::vec3(1.0f, 1.6f, 0.12f));

	translation = glm::translate(glm::vec3(8.9f, -2.4f, -6.9f));

	// Apply Transformation every frame
	model = rotation * scale * translation;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glDrawArrays(GL_TRIANGLES, 0, pyramidMesh.nvertices[2]);
	glUseProgram(programId);

	// Render Laptop Screen -----------------------------------------------------------------------
	glUseProgram(programId);
	glBindVertexArray(pyramidMesh.vao[2]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, laptopScreenTex);
	scale = glm::scale(glm::vec3(4.7f, 4.4f, 0.12f));

	translation = glm::translate(glm::vec3(1.92f, 0.26f, -9.22f));

	// Apply Transformation every frame
	model =  scale * translation;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glDrawArrays(GL_TRIANGLES, 0, pyramidMesh.nvertices[2]);
	glUseProgram(programId);



// Render Laptop Base -----------------------------------------------------------------------
	glUseProgram(programId);
	glBindVertexArray(pyramidMesh.vao[4]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, phoneTex);
	scale = glm::scale(glm::vec3(9.0f, 6.0f, 0.12f));

	translation = glm::translate(glm::vec3(1.0f, -0.3, -7.9f));

	// Apply Transformation every frame
	model = rotation * scale * translation;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glDrawArrays(GL_TRIANGLES, 0, pyramidMesh.nvertices[4]);
	glUseProgram(programId);

	// Render Laptop Top -----------------------------------------------------------------------
	glUseProgram(programId);
	glBindVertexArray(pyramidMesh.vao[4]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, phoneTex);
	scale = glm::scale(glm::vec3(9.0f, 4.4f, 0.12f));

	translation = glm::translate(glm::vec3(9.0f, 1.12, -1.24));
	//rotation = glm::rotate(-90.0f, glm::vec3(0.0f, 0.0f, 0.33f));
	// Apply Transformation every frame
	model =translation * scale;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glDrawArrays(GL_TRIANGLES, 0, pyramidMesh.nvertices[4]);
	glUseProgram(programId);


// Render Pencil Object -----------------------------------------------------------------------
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pencilBaseTex);
	glBindVertexArray(pyramidMesh.vao[3]);
	model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first		
	translation = glm::translate(model, glm::vec3(-4.0f, -0.8f, 2.0f));
	scale = glm::scale(model, glm::vec3(0.2f, 0.03f, 0.2f));
	rotation = glm::rotate(-33.0f, glm::vec3(0.33f, 0.0f, 0.0f));
	model = translation * rotation * scale;

	modelLoc = glGetUniformLocation(programId, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	static_meshes_3D::Cylinder Pen(0.5, 10, 140, true, true, true);
	Pen.render();

	// Render Pencil Eraser -----------------------------------------------------------------------
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screenTex);
	glBindVertexArray(pyramidMesh.vao[3]);
	model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first		
	translation = glm::translate(model, glm::vec3(-4.0f, -0.83f, -0.15f));
	scale = glm::scale(model, glm::vec3(0.2f, 0.03f, 0.2f));
	rotation = glm::rotate(-33.0f, glm::vec3(0.33f, 0.0f, 0.0f));
	model = translation * rotation * scale;
	modelLoc = glGetUniformLocation(programId, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	static_meshes_3D::Cylinder PenMetal(0.5, 20, 5, true, true, true);
	PenMetal.render();


// Render Pencil Eraser -----------------------------------------------------------------------
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, eraserTex);
	glBindVertexArray(pyramidMesh.vao[3]);
	model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first		
	translation = glm::translate(model, glm::vec3(-4.0f, -0.83f, -0.14f));
	scale = glm::scale(model, glm::vec3(0.2f, 0.03f, 0.2f));
	rotation = glm::rotate(-33.0f, glm::vec3(0.33f, 0.0f, 0.0f));
	model = translation * rotation * scale;
	modelLoc = glGetUniformLocation(programId, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	static_meshes_3D::Cylinder PenEraser(0.4, 12, 15, true, true, true);
	PenEraser.render();

	// Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
	glUniform3f(objectColorLoc, objectColors.r, objectColors.g, objectColors.b);
	const glm::vec3 cameraPosition = camera.Position;
	// Main Light
	glUniform3f(lightColorLoc, spotLightColor.r, spotLightColor.g, spotLightColor.b);
	glUniform3f(lightPositionLoc, spotLightPosition.x, spotLightPosition.y, spotLightPosition.z);
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	// Key Light
	glUniform3f(keyLightColorLoc, keyLightColor.r, keyLightColor.g, keyLightColor.b);
	glUniform3f(keyLightPositionLoc, 0.0f, 0.0f, 0.0f);
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);



	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));


	// Lighting shader code
	glUseProgram(lampProgramId);

	glBindVertexArray(pyramidMesh.vao[2]);
	scale = glm::scale(spotLightScale);

	// Rotate the pyramid by an incrementing float value on z-axis
	rotation = glm::rotate(0.2f, glm::vec3(90.0f, 180.0f, 100.0f));

	// Originate the pyramids vectors
	translation = glm::translate(glm::vec3(10.0f, 30.0f, 10.0f));
	model = translation * rotation * scale;
	// Reference matrix uniforms from the Lamp Shader program
	modelLoc = glGetUniformLocation(lampProgramId, "model");
	viewLoc = glGetUniformLocation(lampProgramId, "view");
	projLoc = glGetUniformLocation(lampProgramId, "projection");

	// Pass matrix data to the Lamp Shader program's matrix uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glDrawArrays(GL_TRIANGLES, 0, pyramidMesh.nvertices[2]);



	glUseProgram(programId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, lampTex);
	// Rotate the pyramid by an incrementing float value on z-axis
	//rotation = glm::rotate(20.0f, glm::vec3(90.0f, 180.0f, 100.0f));

	// Originate the pyramids vectors
	scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
	translation = glm::translate(spotLightPosition);
	model = translation * scale;
	// Reference matrix uniforms from the Lamp Shader program
	modelLoc = glGetUniformLocation(lampProgramId, "model");
	viewLoc = glGetUniformLocation(lampProgramId, "view");
	projLoc = glGetUniformLocation(lampProgramId, "projection");

	// Pass matrix data to the Lamp Shader program's matrix uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glBindVertexArray(pyramidMesh.vao[5]);
	glDrawArrays(GL_TRIANGLES, 0, pyramidMesh.nvertices[5]);


	
	// Deactivate the VAO data
	glBindVertexArray(0);

	// swap between buffers, check events from poll
	glfwSwapBuffers(window);
}

// Creation of shader program
void createShader(const char* vertexShader, const char* fragmentShader, GLuint& programId) {
	// Create shader
	programId = glCreateProgram();

	// Initialize vertex and fragment shaders
	GLuint vtxShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrive the shader from compiled code
	glShaderSource(vtxShader, 1, &vertexShader, NULL);
	glShaderSource(fragShader, 1, &fragmentShader, NULL);

	// Compile both vertex and fragment shaders
	glCompileShader(vtxShader);
	glCompileShader(fragShader);

	// Attached compiled shaders to the mesh data
	glAttachShader(programId, vtxShader);
	glAttachShader(programId, fragShader);

	// Link shader to mesh data
	glLinkProgram(programId);
	glUseProgram(programId);
}

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/glew.h>
#include <ASSIMP/mesh.h>
#include <ASSIMP/material.h>
#include <ASSIMP/scene.h>
#include <ASSIMP/Importer.hpp>
#include <ASSIMP/postprocess.h>

#define GLM_FORCE_RADIANS
#include <GLM/glm.hpp>
#include <GLM/ext.hpp>

// Convience function to convert between degrees and radians
const float PI = 3.14159265359f;
float Radians(float degrees) { return degrees * (PI / 180.0f); }
float Degrees(float radians) { return radians * (180.0f / PI); }

// Struct to hold mesh loaded into OpenGL
struct Mesh
{
	unsigned int drawCount = 0;
	unsigned int materialIndex = 0;
	GLuint indexBuffer;
	GLuint vertexBuffer;
	GLuint uvBuffer;
	GLuint vertexArrayObject; 
	bool hasUvs = false;
};

// Struct to hold material loaded into OpenGL
struct Material
{
	glm::vec3 diffuseColor;
	GLuint diffuseTexture;
	bool hasDiffuseTexture;
};

// Struct to hold a loaded model
struct Model
{
std::vector<Mesh*> meshes;
std::vector<Material*> materials;
};

// OpenGL version
const unsigned int GL_VERSION_MAJOR = 3;
const unsigned int GL_VERSION_MINOR = 1;

// Model variables
glm::vec3 modelPosition = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 modelScale = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 modelRotation = glm::vec3(0.0f, 0.0f, 0.0f); // euler angles in radians
glm::mat4 modelMatrix;
const std::string modelFile = "models/Crate.obj";
Model* model;

// Camera variables
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraRotation = glm::vec3(0.0f, 0.0f, 0.0f); // euler angles in radians
float fieldOfView = 1.0f; // field of view in radians (1.0 radian ~= 60 degrees)
float zNear = 0.01f;
float zFar = 1000.0f;
glm::mat4 cameraView;
glm::mat4 cameraProjection;

// Shader variables
GLuint shaderProgram;
GLuint vertexShader;
GLuint fragmentShader;
GLuint vertexAttrib;
GLuint uvAttrib;
GLuint modelMatUniform;
GLuint cameraViewMatUniform;
GLuint cameraProjMatUniform;
GLuint diffuseTextureUniform;
GLuint hasDiffuseTextureUniform;
GLuint diffuseColorUniform;

// Display variables
SDL_Window *window;
SDL_GLContext context;
unsigned int displayWidth = 800;
unsigned int displayHeight = 800;

// Program variables
bool quit = false;
float deltaTime = 0.0f;

void InitialiseSDL(); // SDL_Init()
void CreateWindow(); // Open a new window
void SetGLAttributes(); // Set various GL attribs
void CreateContext(); // Create a OpenGL context to render into
void InitialiseGlew(); // glewInit()
void LoadShader(); // load shader
void LoadModel(); // load model
void Update(float deltaTime); // main update function
void Render(); // main render function
void UnloadModel(); // unload model
void UnloadShader(); // unload shader
void Quit();

int main(int argc, char *argv[])
{
	// Setup
	InitialiseSDL();
	CreateWindow();
	SetGLAttributes();
	CreateContext();
	InitialiseGlew();
	
	// Load shader
	LoadShader();
	
	// Load the model
	LoadModel();
	
	while(!quit)
	{	
		// Frame timing
		unsigned int startTime = SDL_GetTicks();
		
		// Update simulation, then render
		Update(deltaTime);
		Render();

		// Calculate deltatime
		unsigned int endTime = SDL_GetTicks();
		deltaTime = (endTime - startTime) / 1000.0f;
	};
	
	// Unload the model
	UnloadModel();
	
	// Unload shader
	UnloadShader();
	
	// Cleanup
	Quit();
	
	return 0;	
}

void InitialiseSDL()
{
	// Init SDL
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "SDL initialisation failed!" << std::endl;
		exit(1);
	}
	else
	{
		std::cout << "SDL initialised!" << std::endl;
	}
}

void CreateWindow()
{
	// Open a window
	window = SDL_CreateWindow("OpenGL, SDL2 and Assimp Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	
	// Error checking
	if(window == nullptr)
	{
		// Print error
		std::cout << "Error creating window: " << SDL_GetError() << std::endl;
		
		// Quit
		SDL_Quit();
		exit(1);
	}
}

void SetGLAttributes()
{
	// Set GL attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_VERSION_MAJOR); // version major
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_VERSION_MINOR); // version minor
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // core profile
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // double buffering
}

void CreateContext()
{
	// Create GL context
	context = SDL_GL_CreateContext(window);
	
	// Error checking
	if (context == nullptr){
		
		// Print error
		std::cout << "Error creating context: " << SDL_GetError() << std::endl;
		
		// Destroy window
		SDL_DestroyWindow(window);
		
		// Quit
		SDL_Quit();
		exit(1);
	}
}

void InitialiseGlew()
{
	// Modern OpenGL
	glewExperimental = GL_TRUE;

	// Init glew
	GLenum glew = glewInit();

	// Check glew status
	if (glew != GLEW_OK)
	{
		// Error
		std::cout << "Glew initialisation failed!" << std::endl;
		std::cout << glewGetErrorString(glew) << std::endl;
		
		// Exit
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(window);
		SDL_Quit();
		exit(1);
	}
	
	// Print OpenGL info
	std::cout << "Glew initialised!" << std::endl;
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	
	// Enable back face culling with counter-clockwise winding for front faces
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	// Set clear color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
}

void Quit()
{
	// Cleaup OpenGL context and window
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	
	// Quit SDL
	SDL_Quit();
}

std::string LoadShaderFromFile(const std::string& filename)
{
	std::ifstream file;
	std::string source;
	std::string line;

	file.open(filename);

	if (file.is_open())
	{
		while (file.good())
		{
			getline(file, line);
			source.append(line + "\n");
		}
	}
	else
	{
		std::cout << "Couldn't open shader: " << filename << std::endl;
	}

	return source;
}

void LoadShader()
{
	// Create shader program
	shaderProgram = glCreateProgram();
	
	// Load shader source
	std::string vertexSource = LoadShaderFromFile("shaders/shader.vert");
	std::string fragmentSource = LoadShaderFromFile("shaders/shader.frag");
	
	// Create vertex shader
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	
	// Convert source to GLchar*
	const GLchar* vertexSourceGL = vertexSource.c_str();
	GLint vertexSourceLength = vertexSource.length();

	// Compile shader
	glShaderSource(vertexShader, 1, &vertexSourceGL, &vertexSourceLength);
	glCompileShader(vertexShader);
	
	// Check compile status
	GLint vertexStatus;
	GLchar vertexError[1024] = { 0 };

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexStatus);

	if (vertexStatus == GL_FALSE)
	{
		glGetShaderInfoLog(vertexShader, sizeof(vertexError), NULL, vertexError);
		std::cout << "Error compiling vertex shader: " << vertexError << std::endl;
	}
	
	// Create fragment shader
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	
	// Convert source to GLchar*
	const GLchar* fragmentSourceGL = fragmentSource.c_str();
	GLint fragmentSourceLength = fragmentSource.length();

	// Compile shader
	glShaderSource(fragmentShader, 1, &fragmentSourceGL, &fragmentSourceLength);
	glCompileShader(fragmentShader);
	
	// Check compile status
	GLint fragmentStatus;
	GLchar fragmentError[1024] = { 0 };

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentStatus);

	if (fragmentStatus == GL_FALSE)
	{
		glGetShaderInfoLog(fragmentShader, sizeof(fragmentError), NULL, fragmentError);
		std::cout << "Error compiling fragment shader: " << fragmentError << std::endl;
	}
	
	// Attach shaders
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	
	// Link program
	glLinkProgram(shaderProgram);
	
	// Validate program
	glValidateProgram(shaderProgram);
	
	// Use program
	glUseProgram(shaderProgram);
	
	// Store attribs
	vertexAttrib = glGetAttribLocation(shaderProgram, "vertex");
	uvAttrib = glGetAttribLocation(shaderProgram, "uv");
	
	// Store uniforms
	modelMatUniform = glGetUniformLocation(shaderProgram, "model");
	cameraViewMatUniform = glGetUniformLocation(shaderProgram, "cameraView");
	cameraProjMatUniform = glGetUniformLocation(shaderProgram, "cameraProjection");
	diffuseTextureUniform = glGetUniformLocation(shaderProgram, "diffuseTexture");
	hasDiffuseTextureUniform = glGetUniformLocation(shaderProgram, "hasDiffuseTexture");
	diffuseColorUniform = glGetUniformLocation(shaderProgram, "diffuseColor");	
	
}

void LoadModel()
{
	// Use assimp to load a scene from the model file, and apply some post processing. See http://assimp.sourceforge.net/lib_html/postprocess_8h.html for more information.
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(modelFile, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenUVCoords | aiProcess_TransformUVCoords | aiProcess_OptimizeMeshes | aiProcess_FlipUVs);
	
	// Store the loaded model in a model struct
	model = new Model();
	
	if (!scene)
	{
		std::cout << "Error loading model: " << modelFile << std::endl;
	}
	else
	{
		std::cout << "Loaded model file: " << modelFile << std::endl;
		
		// Use shader
		glUseProgram(shaderProgram);

		// Loop through all the meshes in the scene
		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			// Use a MeshStruct to store a mesh
			Mesh* mesh = new Mesh();
			
			// Vectors to store model data whilst we pass to OpenGL
			std::vector<unsigned int> indices;
			std::vector<glm::vec3> vertices;
			std::vector<glm::vec2> uvs;
			
			// Assimp stores indices in faces, these will all be triangles due to the aiProcess_Triangulate flag passed when loading the model
			for (unsigned int j = 0; j < scene->mMeshes[i]->mNumFaces; j++)
			{
				// Push indices back into one long list and count
				indices.push_back(scene->mMeshes[i]->mFaces[j].mIndices[0]);
				indices.push_back(scene->mMeshes[i]->mFaces[j].mIndices[1]);
				indices.push_back(scene->mMeshes[i]->mFaces[j].mIndices[2]);
				mesh->drawCount += 3;
			}
			
			// Generate index buffer
			glGenBuffers(1, &mesh->indexBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->indexBuffer);
			glBufferData(GL_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
			
			// Push vertices and uvs back into their lists
			for (unsigned int j = 0; j < scene->mMeshes[i]->mNumVertices; j++)
			{
				vertices.push_back(glm::vec3(scene->mMeshes[i]->mVertices[j].x, scene->mMeshes[i]->mVertices[j].y, scene->mMeshes[i]->mVertices[j].z));

				// Check the model has uvs
				if (scene->mMeshes[i]->mTextureCoords[0] != NULL)
					uvs.push_back(glm::vec2(scene->mMeshes[i]->mTextureCoords[0][j].x, scene->mMeshes[i]->mTextureCoords[0][j].y));
			}
			
			// Generate vertex buffer
			glGenBuffers(1, &mesh->vertexBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
			
			// Generate uv buffer if we have uvs
			if(uvs.size() > 0)
			{
				mesh->hasUvs = true;
				glGenBuffers(1, &mesh->uvBuffer);
				glBindBuffer(GL_ARRAY_BUFFER, mesh->uvBuffer);
				glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);	
			}
			
			// Store material index
			mesh->materialIndex = scene->mMeshes[i]->mMaterialIndex;

			// Generate vertex array object
			glGenVertexArrays(1, &mesh->vertexArrayObject);
							  
			// Tell OpenGL how to interpret the model data
			glBindVertexArray(mesh->vertexArrayObject);
			
			// Vertex buffer
			glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
			glEnableVertexAttribArray(vertexAttrib);
			glVertexAttribPointer(vertexAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
							  
			// Uv buffer
			if(mesh->hasUvs)
			{
				glBindBuffer(GL_ARRAY_BUFFER, mesh->uvBuffer);
				glEnableVertexAttribArray(uvAttrib);
				glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
			}
							  
			// Index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
							  
			// Unbind everything
			glBindVertexArray(0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			
			// Add to model
			model->meshes.push_back(mesh);
		}

		// Loop through all the material in the scene
		for (unsigned int i = 0; i < scene->mNumMaterials; i++)
		{
			// Store loaded material in material struct
			Material* material = new Material();
			
			// Get the diffuse color of the material
			scene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, material->diffuseColor);

			std::cout << material->diffuseColor.x << ", " << material->diffuseColor.y << ", " << material->diffuseColor.y << std::endl;
			
			// Check if the material has a diffuse texture
			if (scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE) > 0)
			{
				// Generate a texture
				glGenTextures(1, &material->diffuseTexture);
				
				material->hasDiffuseTexture = true;
				
				// Get texture file path
				aiString filename;
				scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &filename);
				
				std::string path = "models/" + std::string(filename.C_Str());
				
				// Load texture using SDL_image
				SDL_Surface* texture = nullptr;
				texture = IMG_Load(path.c_str());
				
				if (texture != nullptr)
				{
					glBindTexture(GL_TEXTURE_2D, material->diffuseTexture);
					
					// Textures have to be passed to OpenGL in the right way depending on format. This is by no means a complete list, and some texture formats might still fail.
					switch (texture->format->format)
					{
					case SDL_PIXELFORMAT_RGB24:
					case SDL_PIXELFORMAT_RGB332:
					case SDL_PIXELFORMAT_RGB444:
					case SDL_PIXELFORMAT_RGB555:
					case SDL_PIXELFORMAT_RGB565:
					case SDL_PIXELFORMAT_RGB888:
						// RGB format
						glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, texture->w, texture->h, 0, GL_RGB, GL_UNSIGNED_BYTE, texture->pixels);
						break;
					case SDL_PIXELFORMAT_RGBA4444:
					case SDL_PIXELFORMAT_RGBA5551:
					case SDL_PIXELFORMAT_RGBA8888:
						// RGBA format
						glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);
						break;
					case SDL_PIXELFORMAT_BGR24:
					case SDL_PIXELFORMAT_BGR555:
					case SDL_PIXELFORMAT_BGR565:
					case SDL_PIXELFORMAT_BGR888:
						// BGR format
						glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, texture->w, texture->h, 0, GL_BGR, GL_UNSIGNED_BYTE, texture->pixels);
						break;
					case SDL_PIXELFORMAT_ABGR1555:
					case SDL_PIXELFORMAT_ABGR4444:
					case SDL_PIXELFORMAT_ABGR8888:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);
						break;
					case SDL_PIXELFORMAT_ARGB1555:
					case SDL_PIXELFORMAT_ARGB2101010:
					case SDL_PIXELFORMAT_ARGB4444:
					case SDL_PIXELFORMAT_ARGB8888:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);
						break;
					default:
						std::cout << "Unknown texture format: " << SDL_GetPixelFormatName(texture->format->format) << std::endl;
						break;
					}
					
					// Enable mipmapping
					glGenerateMipmap(GL_TEXTURE_2D);

					// Set texture parameters.
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repeat wrapping
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repeat wrapping
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // linear mag filtering
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // trilinear min filtering

					// Unbind the texture
					glBindTexture(GL_TEXTURE_2D, 0);

					std::cout << "Loaded texture: " << path << std::endl;					
					
				}
				else
				{
					std::cout << "Failed to load texture: " << path << std::endl;
				}
				
				SDL_FreeSurface(texture);
				texture = nullptr;
			}
			
			// Add to model
			model->materials.push_back(material);
		}

	}
	
	// Cleanup scene
	importer.FreeScene();
}

void UnloadShader()
{
	// Detach and delete vertex shader
	glDetachShader(shaderProgram, vertexShader);
	glDeleteShader(vertexShader);
	
	// Detach and delete fragment shader
	glDetachShader(shaderProgram, fragmentShader);
	glDeleteShader(fragmentShader);
	
	// Delete shader program
	glDeleteProgram(shaderProgram);	
}

void UnloadModel()
{

	// Loop through all the meshes
	for(Mesh* mesh : model->meshes)
	{
		// Delete buffers
		glDeleteBuffers(1, &mesh->indexBuffer);
		glDeleteBuffers(1, &mesh->vertexBuffer);
		if(mesh->hasUvs) glDeleteBuffers(1, &mesh->uvBuffer);
		
		// Delete vertex array
		glDeleteVertexArrays(1, &mesh->vertexArrayObject);
		
		// Delete the mesh object
		delete mesh;
	}
	
	// Loop through all the materials
	for(Material* material : model->materials)
	{
		// Delete diffuse texture
		if(material->hasDiffuseTexture) glDeleteTextures(1, &material->diffuseTexture);
		
		// Delete the material object
		delete material;
	}
	
	// Clear lists
	model->meshes.clear();
	model->materials.clear();
	
}

void Update(float deltaTime)
{
	SDL_Event e;
		
	// Poll for events
	while(SDL_PollEvent(&e))
	{
		// Exit main loop on quit event
		if(e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)) quit = true;
	}

	// Get the current keystate. This must be called after SDL_PollEvents has finished.
	const Uint8* keystate = SDL_GetKeyboardState(NULL);

	// Move camera forward using up/w key
	if (keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_W])
		cameraPosition += glm::vec3(0.0f, 0.0f, -5.0f) * deltaTime;

	// Move camera backward using down/s key
	if (keystate[SDL_SCANCODE_DOWN] || keystate[SDL_SCANCODE_S])
		cameraPosition += glm::vec3(0.0f, 0.0f, 5.0f) * deltaTime;

	// Rotate model using left/a key
	if(keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_A])
		modelRotation += glm::vec3(0.0f, Radians(-45.0f), 0.0f) * deltaTime;

	// Rotate model using right/d key
	if (keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_D])
		modelRotation += glm::vec3(0.0f, Radians(45.0f), 0.0f) * deltaTime;

	// Scale model down using left bracket key
	if (keystate[SDL_SCANCODE_LEFTBRACKET])
		modelScale += glm::vec3(-0.1f) * deltaTime;

	// Scale model up using right bracket key
	if (keystate[SDL_SCANCODE_RIGHTBRACKET])
		modelScale += glm::vec3(0.1f) * deltaTime;
	
	// Recalculate camera projection matrix. This doesn't really need to be recalculated every frame.
	cameraProjection = glm::perspective(fieldOfView, (float)displayWidth / (float)displayHeight, zNear, zFar);
	
	// Recalculate camera position matrix
	cameraView = glm::inverse(glm::translate(cameraPosition) * glm::mat4_cast(glm::quat(cameraRotation)) * glm::scale(glm::vec3(1.0f))); // camera has no scaling
	
	// Recalculate model position matrix
	modelMatrix = glm::translate(modelPosition) * glm::mat4_cast(glm::quat(modelRotation)) * glm::scale(modelScale);
}

void Render()
{
	// Clear color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Use shader
	glUseProgram(shaderProgram);
	
	// Update uniform variables
	glUniformMatrix4fv(modelMatUniform, 1, false, &modelMatrix[0][0]);
	glUniformMatrix4fv(cameraViewMatUniform, 1, false, &cameraView[0][0]);
	glUniformMatrix4fv(cameraProjMatUniform, 1, false, &cameraProjection[0][0]);
	
	// Draw
	for(Mesh* mesh : model->meshes)
	{
		// Get material to draw with
		Material* material = model->materials[mesh->materialIndex];
		
		// Update material uniforms
		glUniform3fv(diffuseColorUniform, 1, &material->diffuseColor[0]);
		glUniform1i(hasDiffuseTextureUniform, material->hasDiffuseTexture);
		
		// Use texture if material has diffuse texture
		if(material->hasDiffuseTexture)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, material->diffuseTexture);
			glUniform1i(diffuseTextureUniform, 0);
		}
		
		// Draw
		glBindVertexArray(mesh->vertexArrayObject);
		glDrawElements(GL_TRIANGLES, mesh->drawCount, GL_UNSIGNED_INT, 0);
		
	}
	
	// Swap buffers
	SDL_GL_SwapWindow(window);
}
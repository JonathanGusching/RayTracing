#ifndef OpenGLcontext_HPP
#define OpenGLcontext_HPP

//standard
#include <iostream>
#include <vector>
#include <string>
#include <ctime> // write to log
#include <chrono>

//OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// headers
#include "ray.hpp"
#include "material.hpp"
#include "shader.hpp"
#include "objects.hpp"
#include "controls.hpp"
#include "camera.hpp"


#define PROGRAM_NAME "Ray Tracing"

class OpenGLcontext
{
	public:

		int SCREEN_WIDTH;
		int SCREEN_HEIGHT;

		GLFWwindow* window;
		/* the CPU representation of the scene */
		SceneManager sceneManager;
    	
    	// Mouse position
    	GLdouble xmouse;
    	GLdouble ymouse;
		
		Camera mainCamera;

		GLuint mainTexture; // texture that contains the entire screen
		
		std::vector<Shader*> shaders;

		/* We create a big quad and map a texture to it using a compute shader */
		GLuint quadIBO;
  		GLuint quadVBO;
  		GLuint quadVAO;
  		GLuint quadTextureID;

		GLuint program;
		GLuint computeProgram;

    	GLuint ssbo;
		/* Functions for the Log system (to file or to terminal) */
		/* The log shows the warnings */
		const void InitializeLogSystemToFile();
		
		enum Prefix {ERROR_MESS, WARNING_MESS, INFO_MESS};
	private:
		template <typename T>
		const void WriteArgs(std::ostream& stream, T message);

		template <typename T, class ...Args>
		const void WriteArgs(std::ostream& stream, T message, Args... arguments);
	public:
		template <class ...Args>
		const void Write(Prefix messageType, std::ostream& stream, Args... arguments);
		const void CloseLog();


		/* Client (CPU) sends the scene information to the server (GPU) */
		const void SendCurrentScene();

		/* All the procedures creating an OpenGL window, creating the quad texture,	/ 
		/  adding shaders (fragment, vertex and compute) and bind them to 			/
		/  the respective program ID 											   */ 
		void CreateWindow(std::string windowName, int screen_width, int screen_height);
		void GenerateTexture();
		void AddShader(const char* file, GLenum type);
		void CreateRenderProgramAndShaders();
		void CreateComputeProgram();
		void PrepareComputeShader();

		/* Rendering procedure called in the loop */
		const void Render();

		/* Camera and movement */
		void RefreshCameraPos();
		void ProcessCameraSpeed();
		void MoveRight();
		void MoveLeft();
		void MoveForward();
		void MoveBackward();

		/* Rendering loop, executed at every frame */
		void Loop();

		/* Procedure to be called before destroying objects */
		void FreeMemory();

		OpenGLcontext();
		~OpenGLcontext();

	private:
		static void FramebufferSizeCallback(GLFWwindow* window, int width, int height); // callback for window resizing
		bool KeyInput();
		float deltaTime=0.0f;
		float lastFrame=0.0f;
		//void key_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods); // callback for key inputs

};
#endif
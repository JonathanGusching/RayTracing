#ifndef OpenGLcontext_HPP
#define OpenGLcontext_HPP

//standard
#include <iostream>
#include <vector>
#include <string>

//OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// headers
#include "ray.hpp"
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
		ObjectManager object_manager;
    	
    	// Mouse position
    	GLdouble xmouse;
    	GLdouble ymouse;
		
		Camera mainCamera;

		GLuint mainTexture; // texture that contains the entire screen
		
		std::vector<Shader*> shaders;

		GLuint quadIBO;
  		GLuint quadVBO;
  		GLuint quadVAO;
  		GLuint quadTextureID;

		GLuint program;
		GLuint computeProgram;

		void AddCube(glm::vec3 pos, GLfloat half_length);
		void AddSphere(glm::vec3 pos, GLfloat radius);

		void CreateWindow(std::string windowName, int screen_width, int screen_height);

		void GenerateTexture();
		void AddShader(const char* file, GLenum type);
		void CreateRenderProgramAndShaders();
		void CreateComputeProgram();
		void Render();
		void PrepareComputeShader();

		void RefreshCameraPos();

		//To do: more optimised?
		void MoveRight();
		void MoveLeft();
		void MoveForward();
		void MoveBackward();

		void Loop();
		//void GenerateVAO();
		//void SendToOpenGL(); // It is important to avoid using immediate rendering with modern OGL in order to fully be able to use the GPU

		OpenGLcontext();
		~OpenGLcontext();

	private:
		static void framebuffer_size_callback(GLFWwindow* window, int width, int height); // callback for window resizing
		//void key_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods); // callback for key inputs

};
#endif
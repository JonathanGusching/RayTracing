#ifndef SHADER_HPP
#define SHADER_HPP

#include <iostream>
#include <fstream>
#include <string>

#include <GL/glew.h>

#include <GL/gl.h>
#include <GLFW/glfw3.h>

std::string LoadSource(const char *filename);

class Shader
{
	public:
		GLuint shader_id;
		GLenum type; // GL_VERTEX_SHADER / GL_FRAGMENT_SHADER / GL_COMPUTE_SHADER

		const char* source;
		void Compile();
		Shader(const char* filename, GLenum type);
		
		// Please avoid copying a Shader object since it might cause ID issues and referencing problems to OpenGL
		Shader(const Shader& shader)
		{
			std::cout << "WARNING : copying shader" << std::endl;
			shader_id=shader.shader_id;
			type=shader.type;
		}
		//Shader(std::string str, GLenum type);
		~Shader();

};


#endif
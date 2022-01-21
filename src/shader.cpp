#include "shader.hpp"

#include <cstring>

#include <GL/gl.h>

std::string LoadSource(const char *filename)
{
	std::string out;
	//Exceptions don't work well with std::iostream

	std::ifstream sourceCode;
	
	//sourceCode.exceptions ( std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit  );
	
	sourceCode.open(filename,std::ifstream::in);
	if(!sourceCode.is_open())
	{
		std::cerr << "Error: Failed to open shader file: " << filename << ": " << std::endl;
		return "";
		
	}
	char c=sourceCode.get();
	while (sourceCode.good()) 
	{
	   	out.push_back(c);
	   	sourceCode.get(c);
	}
  	sourceCode.close();
    return out;
}

void Shader::Compile()
{
	glCompileShader(shader_id); // Compilation
	
	GLint compile_status;
	GLint logsize;
	GLchar* log;

    std::string str;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_status);
	if(compile_status != GL_TRUE)
    {
        /* If cannot compile, write the log in the error stream */
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &logsize);
        
        log=new char[logsize+1];
        if(log == NULL)
        {
            std::cerr << "Unable to allocate memory for compilation error log";
        }
        
        glGetShaderInfoLog(shader_id, logsize, &logsize, log);
        
        std::cerr << "Unable to compile shader " << shader_id << " := "<<std::endl
        << log << std::endl;
        
        delete(log);
        glDeleteShader(shader_id); // once compiled, it is useless   
    }
    std::cout << "Shader " << shader_id << " compiled." << std::endl;

}


Shader::Shader(const char* filename, GLenum arg_type)
{
	type=arg_type;

	shader_id=glCreateShader(type); //Create the shader
	if(shader_id==0)
	{
		std::cerr << "ERROR when creating shader" << std::endl;
	}
	std::string src_string=LoadSource(filename); //Opening source file
	source=src_string.c_str();
	
	glShaderSource(shader_id,1,&source,0); //Loading from source
	
	Compile();

}

Shader::~Shader()
{
	glDeleteShader(shader_id);
}
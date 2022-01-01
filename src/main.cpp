#include <iostream>
#include <vector>
#include <string>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "shader.hpp"

#include "objects.hpp"
#include "controls.hpp"
#include "camera.hpp"

#include "OpenGLcontext.hpp"


int main()
{
    OpenGLcontext context;

    context.CreateWindow(PROGRAM_NAME,1920,1080);

    //glEnable(GL_CULL_FACE);

    // Creating objects
    //Cube cube(0.2,glm::vec3{0.,0.,0.0});
    //Triangle tri =Triangle();
    //Triangle tri2 =Triangle(-0.95);
    //Rectangle rec=Rectangle();

    // adding objects to the object manager
    //context.object_manager.AddObject(GL_STATIC_DRAW,tri2);
    //context.object_manager.AddObject(GL_STATIC_DRAW,rec);

    //Adding the shaders
    context.AddShader("simple_vertex.vert", GL_VERTEX_SHADER);
    context.AddShader("simple_color.frag", GL_FRAGMENT_SHADER);
    
    context.AddShader("simple_compute.shader", GL_COMPUTE_SHADER);
    

    context.GenerateTexture();
    
    //context.SendToOpenGL(); // preparing the GPU memory for the objects, their format, attributes, etc...
    //context.GenerateVAO(); // creating the GPU objects
    
    context.CreateRenderProgramAndShaders(); // create the program and the shaders in the context
    context.CreateComputeProgram();

    /* ----- Run Compute shader ----- */
    context.PrepareComputeShader();

    context.Loop();

    return 0;
}
#include <iostream>
#include <vector>
#include <string>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "shader.hpp"

#include "material.hpp"
#include "objects.hpp"
#include "controls.hpp"
#include "camera.hpp"

#include "OpenGLcontext.hpp"


int main()
{
    
    SceneManager sm;
    
    Object obj=Object(glm::vec3(0.0,1.0,2.0));
    Material mat=Material(0.5,0.4,0.0,1.0, glm::vec3(1.0,1.0,1.0));
    Cube cube=Cube(glm::vec3(0.0,0.0,0.0), glm::vec3(10.0,10.0,10.0), mat );
    sm.currentScene.AddObject(obj);
    sm.currentScene.AddObject(cube);
    sm.ExportScene("../scenes/test.xml");
    
    OpenGLcontext context;
    
    context.InitializeLogSystemToFile();
    context.CreateWindow(PROGRAM_NAME,1920,1080);

    //Adding the shaders
    context.AddShader("shaders/simple_vertex.vert", GL_VERTEX_SHADER);
    context.AddShader("shaders/simple_color.frag", GL_FRAGMENT_SHADER);
    
    context.AddShader("shaders/ray_tracing.shader", GL_COMPUTE_SHADER);
    

    context.GenerateTexture();
    
    //context.SendToOpenGL(); // preparing the GPU memory for the objects, their format, attributes, etc...
    //context.GenerateVAO(); // creating the GPU objects
    
    context.CreateRenderProgramAndShaders(); // create the program and the shaders in the context
    context.CreateComputeProgram();

    /* ----- Run Compute shader ----- */
    context.SendCurrentScene();
    context.PrepareComputeShader();


    context.Loop();

    return 0;
}
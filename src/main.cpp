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
    Object obj1=Object(glm::vec3(7.0,4.0,2.0));
    
    Material mat=Material(0.5,0.4,0.0,1.0, glm::vec3(1.0,0.0,0.0));
    Material mat1=Material(0.5,0.4,0.0,1.0, glm::vec3(1.0,0.0,1.0));

    Cube cube=Cube(glm::vec3(0.0,0.0,0.0), glm::vec3(100.0,100.0,100.0), mat );
    Cube cube1=Cube(glm::vec3(0.0,19.0,0.0), glm::vec3(10.6,49.0,10.0), mat );
    Cube cube2=Cube(glm::vec3(0.0,19.0,49.0), glm::vec3(10.6,10.0,10.0), mat );
    
    Sphere sphere=Sphere(glm::vec3(0.0,0.0,0.0), 50.0, mat);
    Sphere sphere2=Sphere(glm::vec3(120.0,0.0,0.0), 70.0, mat1);
    Sphere sphere3=Sphere(glm::vec3(-140.0,0.0,0.0), 90.0, mat1);

    Triangle tri=Triangle(glm::vec3(0.0,0.0,0.0),glm::vec3(100.0,100.0,0.0),glm::vec3(-100.0,0.0,0.0),mat);

    Cylinder cylinder=Cylinder(glm::vec3(0.0,20.0,20.0), glm::vec3(0.0, 400.0, 20.0), 80.0, mat);
    
    //sm.currentScene.AddObject(cube1);
    //sm.currentScene.AddObject(cube);
    //sm.currentScene.AddObject(obj);
    //sm.currentScene.AddObject(obj1);
    sm.currentScene.AddObject(sphere);
    sm.currentScene.AddObject(sphere2);
    sm.currentScene.AddObject(sphere3);
    sm.currentScene.AddObject(tri);
    sm.currentScene.AddObject(cylinder);

    sm.ExportScene("../scenes/test.xml");
    sm.ImportScene("../scenes/test.xml");
    
    OpenGLcontext context;
    context.sceneManager=sm;
    
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
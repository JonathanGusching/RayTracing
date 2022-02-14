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
    
    /* Initialize a few example objects */
    Material mat=Material(0.5,0.4,0.0,1.0, glm::vec3(1.0,0.0,0.0));
    Material mat1=Material(0.5,0.4,0.0,1.0, glm::vec3(1.0,0.0,1.0));
    Material mat2=Material(0.3,0.4,0.0,1.0, glm::vec3(0.0,0.0,0.4));

    Cube cube=Cube(glm::vec3(50.0,10.0,0.0), glm::vec3(100.0,60.0,50.0), mat1 );
    Cube cube1=Cube(glm::vec3(0.0,19.0,0.0), glm::vec3(10.6,49.0,10.0), mat );
    Cube cube2=Cube(glm::vec3(0.0,19.0,49.0), glm::vec3(10.6,10.0,10.0), mat );
    
    Sphere sphere=Sphere(glm::vec3(0.0,0.0,0.0), 50.0, mat);
    Sphere sphere2=Sphere(glm::vec3(120.0,0.0,0.0), 70.0, mat1);
    Sphere sphere3=Sphere(glm::vec3(-140.0,0.0,0.0), 90.0, mat1);

    Triangle tri=Triangle(glm::vec3(0.0,0.0,0.0),glm::vec3(100.0,100.0,0.0),glm::vec3(-100.0,0.0,0.0),mat);

    Cylinder cylinder=Cylinder(glm::vec3(0.0,20.0,20.0), glm::vec3(0.0, 400.0, 20.0), 80.0, mat);
    

    /* Initialize the OpenGL context */
    OpenGLcontext context;


    /* Adding objects to an empty scene */
    //context.sceneManager.currentScene.AddObject(sphere);
    //context.sceneManager.currentScene.AddObject(sphere2);
    context.sceneManager.currentScene.AddObject(sphere3);
    context.sceneManager.currentScene.AddObject(tri);
    context.sceneManager.currentScene.AddObject(cylinder);
    context.sceneManager.currentScene.AddObject(cube);
    context.sceneManager.currentScene.AddObject(cube1);


    /* if you want to save the scene you just created */
    context.sceneManager.ExportScene("../scenes/test.xml");
    context.sceneManager.ImportScene("../scenes/test.xml"); // if you want to import it
    
    context.InitializeLogSystemToFile();
    context.CreateWindow(PROGRAM_NAME,1920,1080);

    
    /* Without the shaders, nothing will be rendered */
    context.AddShader("shaders/simple_vertex.vert", GL_VERTEX_SHADER);
    context.AddShader("shaders/simple_color.frag", GL_FRAGMENT_SHADER);
    context.AddShader("shaders/ray_tracing.shader", GL_COMPUTE_SHADER);
    

    /* Generate the quad */
    context.GenerateTexture();
    
    context.CreateRenderProgramAndShaders(); // create the program and the shaders in the context
    context.CreateComputeProgram();

    /* ----- Run Compute shader ----- */
    context.SendCurrentScene();
    context.PrepareComputeShader();


    context.Loop();

    return 0;
}
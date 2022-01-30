#include "OpenGLcontext.hpp"

#define VERTEX_BYTE_SIZE 3*sizeof(GLfloat)

// callback function to resize the window
void OpenGLcontext::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void CursorCallBack(GLFWwindow* window, double x, double y)
{
    OpenGLcontext* context = static_cast<OpenGLcontext*>(glfwGetWindowUserPointer(window));

    if(context->mainCamera.first_time)
    {
        context->mainCamera.last_x=x;
        context->mainCamera.last_y=y;

        context->mainCamera.first_time=false;
    }
    float sensitivity=0.1f;
    context->mainCamera.yaw+= glm::radians(sensitivity*(context->mainCamera.last_y-y));
    context->mainCamera.pitch+= glm::radians(sensitivity*(x-context->mainCamera.last_x));

    context->mainCamera.last_x=x;
    context->mainCamera.last_y=y;
    
    /* Check rotations */
    context->mainCamera.direction.z = cos(context->mainCamera.yaw) * cos(context->mainCamera.pitch);
    context->mainCamera.direction.x = sin(-context->mainCamera.pitch);
    context->mainCamera.direction.y = sin(context->mainCamera.yaw)*cos(context->mainCamera.pitch);
    
    //context->mainCamera.direction=context->mainCamera.direction-context->mainCamera.centerPos;

    context->mainCamera.direction=glm::normalize(context->mainCamera.direction);
    
    context->RefreshCameraPos();
}

/* input pooling */
bool OpenGLcontext::KeyInput()
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        return false;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        MoveRight();
    }
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        MoveLeft();
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        MoveForward();
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        MoveBackward();
    }
    RefreshCameraPos();
    return true;            
}

void OpenGLcontext::MoveRight()
{
    mainCamera.Translate(glm::vec3(0.3f,0.0f,0.0f)* mainCamera.cameraSpeed);
}
void OpenGLcontext::MoveLeft()
{
    mainCamera.Translate(glm::vec3(-0.3f,0.0f,0.0f)* mainCamera.cameraSpeed);

}
void OpenGLcontext::MoveForward()
{
    mainCamera.Translate(mainCamera.direction * mainCamera.cameraSpeed);
}
void OpenGLcontext::MoveBackward()
{
    mainCamera.Translate(-mainCamera.direction * mainCamera.cameraSpeed);
}

void OpenGLcontext::ProcessCameraSpeed()
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;  
    mainCamera.cameraSpeed=mainCamera.cameraSpeed * deltaTime;
}

const void OpenGLcontext::InitializeLogSystemToFile()
{
  freopen( "../logs/log.txt", "w", stderr );
  std::cout << "Error logs written in ../logs/log.txt" << std::endl;
}

// Need to use C functions to be able to get rid of the last \n character in ctime...
const void OpenGLcontext::WriteToLog(std::string message)
{
    char timeCode[30];
    std::time_t t=std::time(nullptr);
    std::strftime(timeCode, 30, "%x %X",std::localtime(&t));
    std::cerr << "(ERROR) " << "[" << timeCode << "] " << message << std::endl; 
}
const void OpenGLcontext::SendCurrentScene()
{
    glUniform1i(glGetUniformLocation(computeProgram, "REFLECTION_NUMBER"), 10);
}


void OpenGLcontext::CreateWindow(std::string windowName, int screen_width, int screen_height)
{
    SCREEN_WIDTH=screen_width;
    SCREEN_HEIGHT=screen_height;

    /* Initialize the library */
    if (!glfwInit())
    {
        WriteToLog("Could not initialize GLFW");
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, windowName.c_str(), NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        WriteToLog("Could not create GLFW window");
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    //Initializing the Viewport
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Callback function to resize window
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    // Callback function for mouse
    glfwSetWindowUserPointer(window, this); // need to access some members of the class in the callback function
    
    // For a smoother movement, we prefer using pooling than a callback fonction

    std::cout << "Window initialized" << std::endl;
    
    /* INITIALIZING GLEW */
    GLenum err=glewInit();
    if(GLEW_OK!=err)
    {
        std::string message="Error while initializing GLEW: ";
        auto errorMessage=std::string((char*)glewGetErrorString(err));
        message+=errorMessage;
        WriteToLog(message);
    }
    else
    {
        std::cout << "GLEW initialized : " << glewGetString(GLEW_VERSION) << std::endl;
    }

    GLint max_vert_attrib;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,&max_vert_attrib);
    std::cout<< "Vertex attribute limit: "<<  max_vert_attrib << " attributes." << std::endl;

    glfwSetCursorPosCallback(window, CursorCallBack);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN|GLFW_CURSOR_DISABLED);

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

}

typedef struct _COORDS_ {
  GLfloat x;
  GLfloat y;
  GLfloat s;
  GLfloat t;
} CoordinatesSet;

typedef struct _RGBA_ {
  GLfloat Red;
  GLfloat Green;
  GLfloat Blue;
  GLfloat Alpha;
} RGBAValues;

void OpenGLcontext::GenerateTexture() {

      /* ----- Quad Context ----- */
    RGBAValues * quadTexture = new RGBAValues[SCREEN_WIDTH * SCREEN_HEIGHT];
    CoordinatesSet * quadVertex = new CoordinatesSet[4];
    unsigned short int * quadIndex = new unsigned short int[3];

    quadVertex[0].x = -1.0f;
    quadVertex[0].y = 1.0f;

    quadVertex[1].x = -1.0f;
    quadVertex[1].y = -1.0f;

    quadVertex[2].x = 1.0f;
    quadVertex[2].y = 1.0f;

    quadVertex[3].x = 1.0f;
    quadVertex[3].y = -1.0f;

    quadIndex[0] = 0;
    quadIndex[1] = 1;
    quadIndex[2] = 2;
    quadIndex[3] = 3;

    quadVertex[0].s = 0.0f;
    quadVertex[0].t = 0.0f;
    quadVertex[1].s = 0.0f;
    quadVertex[1].t = 1.0f;
    quadVertex[2].s = 1.0f;
    quadVertex[2].t = 0.0f;
    quadVertex[3].s = 1.0f;
    quadVertex[3].t = 1.0f;

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        for (int y = 0; y< SCREEN_HEIGHT; y++) {
            quadTexture[x + y * SCREEN_WIDTH].Red = 1.0f;
            quadTexture[x + y * SCREEN_WIDTH].Green = .5f;
            quadTexture[x + y * SCREEN_WIDTH].Blue = .0f;
            quadTexture[x + y * SCREEN_WIDTH].Alpha = 1.0f;
        }
    }

    /* ----- Render Context ----- */

    /* Creating texture */
    glGenTextures(1, &quadTextureID);
    glBindTexture(GL_TEXTURE_2D, quadTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenBuffers(1, &quadIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(unsigned short int), quadIndex, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CoordinatesSet) * 4, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CoordinatesSet) * 4, quadVertex);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIBO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(sizeof(GLfloat) * 2));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    delete quadTexture;
    delete quadVertex;
    delete quadIndex;
}



void OpenGLcontext::AddShader(const char* file, GLenum type)
{
    shaders.push_back((new Shader(file, type)));
}

/*
Shader objects may be attached to program objects before source code has
been loaded into the shader object, or before the shader object has been compiled
or specialized. Multiple shader objects of the same type may be attached to a
single program object, and a single shader object may be attached to more than
one program object.

-Source: Khronos.org
*/
void OpenGLcontext::CreateRenderProgramAndShaders()
{
    if((program=glCreateProgram())==0)
    {
        WriteToLog("Could not create render program");
        return;
    }

    std::cout << "Render Program created, ID= " << program << std::endl;

    for(GLuint i=0; i<shaders.size(); i++)
    {
        GLenum error=0;
        if(shaders[i]->type!=GL_COMPUTE_SHADER)
        {
            std::cout << "ID: " << shaders[i]->shader_id << std::endl;
            glAttachShader(program, shaders[i]->shader_id);

            error=glGetError();
            switch(error)
            {
                case(GL_INVALID_VALUE):WriteToLog("Program or shader isn't a value generated by OpenGL"); break;
                case(GL_INVALID_OPERATION):WriteToLog("Program or shader is not a program/shader object"); break;
            
            }
        }
    }


    glBindFragDataLocation(program, 0, "color");
    glUniform1i(glGetUniformLocation(program, "srcTex"), 0);
    
    glLinkProgram(program);
    glUseProgram(program);

    GLuint vertArray;
    glGenVertexArrays(1, &vertArray);
    glBindVertexArray(vertArray);

    GLuint posBuf;
    glGenBuffers(1, &posBuf);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf);
    float data[] = {
        -1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8, data, GL_STREAM_DRAW);
    GLint posPtr = glGetAttribLocation(program, "pos");
    glVertexAttribPointer(posPtr, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posPtr);

}
void OpenGLcontext::CreateComputeProgram()
{

    if((computeProgram=glCreateProgram())==0)
    {
        WriteToLog("Could not create compute program");
        return;
    }

    std::cout << "Compute Program created, ID= " << computeProgram << std::endl;

    for(GLuint i=0; i<shaders.size(); i++)
    {
        GLenum error=0;
        if(shaders[i]->type==GL_COMPUTE_SHADER)
        {
            glAttachShader(computeProgram, shaders[i]->shader_id);
            
            error=glGetError();
            switch(error)
            {
                case(GL_INVALID_VALUE):WriteToLog("Program or shader isn't a value generated by OpenGL"); break;
                case(GL_INVALID_OPERATION):WriteToLog("Program or shader is not a program/shader object"); break;
            }
        }
    }

    glLinkProgram(computeProgram);
    glUseProgram(computeProgram);
    glUniform1i(glGetUniformLocation(computeProgram, "destTex"), 0);
    glUniform3f(glGetUniformLocation(computeProgram, "camera_pos"), 0.0, 0.0, 0.0);
}



const void OpenGLcontext::Render()
{

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT);

    // Update the texture
    glUseProgram(computeProgram);
    glDispatchCompute(SCREEN_WIDTH/16, SCREEN_HEIGHT/16, 1);

    // Actual Render
    glUseProgram(program);    
    
    glBindVertexArray(quadVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, quadTextureID);

    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (GLvoid *)0);

    glBindTexture(GL_TEXTURE_2D, 0);
    
    /* Poll and process events */
    glfwPollEvents();
    
    if(glGetError()!=0)
    {
        WriteToLog("OpenGL RENDERING ERROR");
    }

    /* Double buffer : Swap front and back buffers */
    glfwSwapBuffers(window);

    glUseProgram(0);
}

void OpenGLcontext::PrepareComputeShader()
{
    glUseProgram(computeProgram);

    glBindTexture(GL_TEXTURE_2D, quadTextureID);
    glBindImageTexture(0, quadTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    
    glDispatchCompute(SCREEN_WIDTH/16, SCREEN_HEIGHT/16, 1);
    
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void OpenGLcontext::RefreshCameraPos()
{
    glUseProgram(computeProgram);
    glUniform3f(glGetUniformLocation(computeProgram, "camera_pos"), mainCamera.centerPos.x, mainCamera.centerPos[1], mainCamera.centerPos[2]);
    glUniform2f(glGetUniformLocation(computeProgram, "angle_xy"), mainCamera.yaw, mainCamera.pitch);
    glUniform3f(glGetUniformLocation(computeProgram, "camera_direction"), mainCamera.direction[0], mainCamera.direction[1], mainCamera.direction[2]);
    
}

void OpenGLcontext::Loop()
{
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        if(KeyInput())
        {
            glUseProgram(computeProgram);

            Render();
        }
        else
        {
            break;
        }
    }
}

void OpenGLcontext::FreeMemory()
{
    for(auto i=0;i<shaders.size();i++)
    {
        delete(shaders[i]);
    }

    glDeleteProgram(program);
    glfwTerminate();
}

OpenGLcontext::OpenGLcontext()
{
}

OpenGLcontext::~OpenGLcontext()
{
    FreeMemory();	
}
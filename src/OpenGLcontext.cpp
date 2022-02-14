#include "OpenGLcontext.hpp"

// callback function to resize the window
void OpenGLcontext::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Callback function computing the angle depending on the camera position and mouvement
void CursorCallBack(GLFWwindow* window, double x, double y)
{
    OpenGLcontext* context = static_cast<OpenGLcontext*>(glfwGetWindowUserPointer(window));

    if(context->mainCamera.first_time)
    {
        context->mainCamera.last_x=x;
        context->mainCamera.last_y=y;

        context->mainCamera.first_time=false;
    }
    float sensitivity=context->mainCamera.sensitivity;
    context->mainCamera.yaw-= glm::radians(sensitivity*(context->mainCamera.last_y-y));
    context->mainCamera.pitch-= glm::radians(sensitivity*(x-context->mainCamera.last_x));

    context->mainCamera.last_x=x;
    context->mainCamera.last_y=y;
    
    /* Forward direction */
    context->mainCamera.direction.z = cos(context->mainCamera.yaw) * cos(context->mainCamera.pitch);
    context->mainCamera.direction.x = sin(-context->mainCamera.pitch);
    context->mainCamera.direction.y = sin(context->mainCamera.yaw)*cos(context->mainCamera.pitch);
    
    //Technically it's already normalized
    //context->mainCamera.direction=glm::normalize(context->mainCamera.direction);
    
    /* Left vector is just a permutation of the direction */
    context->mainCamera.left.x = -cos(context->mainCamera.yaw) * cos(context->mainCamera.pitch);
    context->mainCamera.left.z = sin(-context->mainCamera.pitch);
    context->mainCamera.left.y = sin(context->mainCamera.yaw)*cos(context->mainCamera.pitch);

    context->RefreshCameraPos();
}

/* input pooling */
bool OpenGLcontext::KeyInput()
{
    bool sprint=false;
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        sprint=true;
    }
    else if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
    {
        sprint=false;
    }
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
        MoveForward(sprint);
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        MoveBackward(sprint);
    }

    RefreshCameraPos();
    return true;            
}

/* Move the camera from the actual position */
void OpenGLcontext::MoveRight()
{
    mainCamera.Translate(-mainCamera.left* mainCamera.cameraSpeed*0.5f);
}
void OpenGLcontext::MoveLeft()
{
    mainCamera.Translate(mainCamera.left * mainCamera.cameraSpeed*0.5f);
}
void OpenGLcontext::MoveForward(bool sprint)
{
    if(sprint)
        mainCamera.Translate(mainCamera.direction * (mainCamera.cameraSpeed+mainCamera.cameraSpeed*1.75f));
    else
        mainCamera.Translate(mainCamera.direction * mainCamera.cameraSpeed);
}
void OpenGLcontext::MoveBackward(bool sprint)
{
    if(sprint)
        mainCamera.Translate(-mainCamera.direction * (mainCamera.cameraSpeed+mainCamera.cameraSpeed*1.5f));
    else
        mainCamera.Translate(-mainCamera.direction * mainCamera.cameraSpeed);
}

/* Takes the lag into account (deltaTime elapsed) */
void OpenGLcontext::ProcessCameraSpeed()
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;  
    mainCamera.cameraSpeed=mainCamera.cameraSpeed * deltaTime;
}

/* Create the log file*/
const void OpenGLcontext::InitializeLogSystemToFile()
{
  freopen( "../logs/log.txt", "w", stderr );
  Write(INFO_MESS, std::cout, "Error logs written in ../logs/log.txt");
}

// Need to use C functions to be able to get rid of the last \n character in ctime...
template <typename T>
const void OpenGLcontext::WriteArgs(std::ostream& stream, T message)
{
    stream << message << std::endl;
}

template <typename T, class ...Args>
const void OpenGLcontext::WriteArgs(std::ostream& stream, T message, Args... arguments)
{
    stream << message;
    WriteArgs(stream, arguments...);
}

/* The generic message writing function */
template <class ...Args>
const void OpenGLcontext::Write(Prefix messageType, std::ostream& stream, Args... arguments)
{
    char timeCode[30];
    std::time_t t=std::time(nullptr);
    std::strftime(timeCode, 30, "%x %X",std::localtime(&t));
    switch(messageType)
    {
        case(ERROR_MESS):stream << "(ERROR) " << "[" << timeCode << "] "; break;
        case(WARNING_MESS):stream << "(WARNING) " << "[" << timeCode << "] "; break;
        case(INFO_MESS):stream << "(INFORMATION) " << "[" << timeCode << "] "; break;
        default:stream << "(OTHER) " << "[" << timeCode << "] "; break;
    }
    WriteArgs(stream, arguments...);
   
}

/* Send the scene to render to OpenGL, don't call it often */
const void OpenGLcontext::SendCurrentScene()
{
    GLintptr offset=0;
    int cptSphere=0;
    int cptCube=0;
    int cptTriangle=0;
    int cptCylinder=0;

    /* An unoptimized way to count objects before looping on each of them... only used to generate the size of each buffer
    To be improved */
    for(Object* object: sceneManager.currentScene.objects)
    {
        if(object->Classname() == "Sphere")
        {
            cptSphere++;
        }
        else if(object->Classname()=="Cube")
        {
            cptCube++;
        }
        else if(object->Classname()=="Triangle")
        {
            cptTriangle++;
        }
        else if(object->Classname()=="Cylinder")
        {
            cptCylinder++;
        }
    }

    glUniform1i(glGetUniformLocation(computeProgram, "REFLECTION_NUMBER"), 8);

    /* SENDING EACH TYPE OF PRIMITIVE OBJECTS INDIVIDUALLY */
    /* SPHERES */
    glGenBuffers(1, &ssboSphere); // Generate buffer

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboSphere); // Bind it (to use it)
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4 + 44 * cptSphere, NULL, GL_DYNAMIC_DRAW); // Generate a storage area (cannot use sizeof(class) because there are extra bytes), hence the static constant
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0 , 4 ,&(cptSphere)); // First element of the buffer is the number of objects (here: spheres)
    offset+=4;
            
    for(Object* object : sceneManager.currentScene.objects)
    {
        if(object->Classname()=="Sphere")
            object->ToBuffer(offset); // otherwise we add all the float values (vertices, material, etc...)
    }
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboSphere);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind


    offset=0;
    /* TRIANGLES */
    glGenBuffers(1, &ssboTriangle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboTriangle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4 + 64 * cptTriangle, NULL, GL_DYNAMIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0 , 4 ,&(cptTriangle));
    offset+=4;
            
    for(Object* object : sceneManager.currentScene.objects)
    {
        if(object->Classname()=="Triangle")
            object->ToBuffer(offset);
    }
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboTriangle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    /* CYLINDERS */
    offset=0;
    glGenBuffers(1, &ssboCylinder);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCylinder);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4 + 56 * cptCylinder, NULL, GL_DYNAMIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0 , 4 ,&(cptCylinder));
    offset+=4;
            
    for(Object* object : sceneManager.currentScene.objects)
    {
        if(object->Classname()=="Cylinder")
            object->ToBuffer(offset);
    }
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboCylinder);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    /* CUBES */
    offset=0;
    glGenBuffers(1, &ssboCube);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCube);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4 + 52 * cptCube, NULL, GL_DYNAMIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0 , 4 ,&(cptCube));
    offset+=4;
            
    for(Object* object : sceneManager.currentScene.objects)
    {
        if(object->Classname()=="Cube")
            object->ToBuffer(offset);
    }
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssboCube);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

}

/* Create the window context */
void OpenGLcontext::CreateWindow(std::string windowName, int screen_width, int screen_height)
{
    SCREEN_WIDTH=screen_width;
    SCREEN_HEIGHT=screen_height;

    /* Initialize the library */
    if (!glfwInit())
    {
        Write(ERROR_MESS, std::cout, "Could not initialize GLFW");
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, windowName.c_str(), NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        Write(ERROR_MESS, std::cout, "Could not create GLFW window");
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

    Write(INFO_MESS, std::cout, "Window initialized");
    
    /* INITIALIZING GLEW */
    GLenum err=glewInit();
    if(GLEW_OK!=err)
    {
        Write(ERROR_MESS, std::cerr, "Error while initializing GLEW: ",glewGetErrorString(err) );
    }
    else
    {
        Write(INFO_MESS, std::cout, "GLEW initialized : ",glewGetString(GLEW_VERSION) );
    }

    GLint max_vert_attrib;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,&max_vert_attrib);
    Write(INFO_MESS, std::cout, "Vertex attribute limit: ",  max_vert_attrib, " attributes.");

    glfwSetCursorPosCallback(window, CursorCallBack);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN|GLFW_CURSOR_DISABLED);

    Write(INFO_MESS, std::cout, "OpenGL version: ", glGetString(GL_VERSION));
    Write(INFO_MESS, std::cout,"GLSL version: ", glGetString(GL_SHADING_LANGUAGE_VERSION));

}

typedef struct _COORDS_ {
  float x;
  float y;
  float s;
  float t;
} CoordinatesSet;

typedef struct _RGBA_ {
  float Red;
  float Green;
  float Blue;
  float Alpha;
} RGBAValues;

/* The complex bit to generate a screen quad and map the texture to it*/
void OpenGLcontext::GenerateTexture() {

      /* ----- Quad Context ----- */
    RGBAValues * quadTexture = new RGBAValues[SCREEN_WIDTH * SCREEN_HEIGHT];
    CoordinatesSet * quadVertex = new CoordinatesSet[4];
    unsigned short int * quadIndex = new unsigned short int[4];

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

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid *)(sizeof(float) * 2));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    delete quadTexture;
    delete quadVertex;
    delete quadIndex;
}


/* Adding a shader to the list */
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
        Write(WARNING_MESS, std::cout, "Could not create render program");
        return;
    }

    std::cout << "Render Program created, ID= " << program << std::endl;

    for(GLuint i=0; i<shaders.size(); i++)
    {
        GLenum error=0;
        if(shaders[i]->type!=GL_COMPUTE_SHADER)
        {
            glAttachShader(program, shaders[i]->shader_id);
            error=glGetError();
            switch(error)
            {
                case(GL_INVALID_VALUE):Write(WARNING_MESS, std::cout, "Program or shader isn't a value generated by OpenGL"); break;
                case(GL_INVALID_OPERATION):Write(WARNING_MESS, std::cout, "Program or shader is not a program/shader object"); break;
            
            }
        }
    }

    // Binding default values
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
/* The compute program, that has to ray trace and everything */
void OpenGLcontext::CreateComputeProgram()
{

    if((computeProgram=glCreateProgram())==0)
    {
        Write(ERROR_MESS, std::cerr, "Could not create compute program");
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
                case(GL_INVALID_VALUE):Write(WARNING_MESS, std::cout, "Program or shader isn't a value generated by OpenGL"); break;
                case(GL_INVALID_OPERATION):Write(WARNING_MESS, std::cout, "Program or shader is not a program/shader object"); break;
            }
        }
    }

    glLinkProgram(computeProgram);
    glUseProgram(computeProgram);
    glUniform1i(glGetUniformLocation(computeProgram, "destTex"), 0);
    glUniform3f(glGetUniformLocation(computeProgram, "camera_pos"), 0.0, 0.0, 0.0);
}


/* Called every frame to render the actual screen */
const void OpenGLcontext::Render()
{
    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT);

    // Update the texture
    glUseProgram(computeProgram);

    // Rendering all the SSBOs one by one.
    // That is very unoptimized and can be improved (By using one SSBO divided into several blocks, for example?)
    int ssbo_binding = 1;
    int block_index = glGetProgramResourceIndex(computeProgram, GL_SHADER_STORAGE_BLOCK, "sphereLayout");
    glShaderStorageBlockBinding(computeProgram, block_index, ssbo_binding );
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo_binding, ssboSphere);
    
    ssbo_binding=2;
    block_index = glGetProgramResourceIndex(computeProgram, GL_SHADER_STORAGE_BLOCK, "triangleLayout");
    glShaderStorageBlockBinding(computeProgram, block_index, ssbo_binding );
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo_binding, ssboTriangle);

    ssbo_binding=3;
    block_index = glGetProgramResourceIndex(computeProgram, GL_SHADER_STORAGE_BLOCK, "cylinderLayout");
    glShaderStorageBlockBinding(computeProgram, block_index, ssbo_binding );
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo_binding, ssboCylinder);

    ssbo_binding=4;
    block_index = glGetProgramResourceIndex(computeProgram, GL_SHADER_STORAGE_BLOCK, "cubeLayout");
    glShaderStorageBlockBinding(computeProgram, block_index, ssbo_binding );
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo_binding, ssboCube);

    // calling the GPU cores to do all the operations.
    glDispatchCompute(SCREEN_WIDTH/16, SCREEN_HEIGHT/16, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


    
    // Actual Render
    glUseProgram(program);
    
    glBindVertexArray(quadVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, quadTextureID);

    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (GLvoid *)0);

    glBindTexture(GL_TEXTURE_2D, 0);
    
    /* Poll and process events */
    glfwPollEvents();

    /* Double buffer : Swap front and back buffers */
    glfwSwapBuffers(window);

    glUseProgram(0);
}

/* Bind the texture */
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

/* Send the camera to GPU */
void OpenGLcontext::RefreshCameraPos()
{
    glUseProgram(computeProgram);
    glUniform3f(glGetUniformLocation(computeProgram, "camera_pos"), mainCamera.centerPos.x, mainCamera.centerPos[1], mainCamera.centerPos[2]);
    glUniform2f(glGetUniformLocation(computeProgram, "angle_xy"), mainCamera.yaw, mainCamera.pitch);
    glUniform3f(glGetUniformLocation(computeProgram, "camera_direction"), mainCamera.direction[0], mainCamera.direction[1], mainCamera.direction[2]);
    
}

/* If the user wants to add things to the CPU processing, it's here! It's the "game loop" */
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

/* Don't forget to avoid memory leaks */
void OpenGLcontext::FreeMemory()
{
    for(long unsigned int i=0;i<shaders.size();i++)
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
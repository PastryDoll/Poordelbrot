//
//  main.cpp
//  GLFWtest
//
//  Created by Caio on 08/07/2023.
//
//
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#define ASSERT(x) if (!(x)) __builtin_debugtrap();
#define GLCall(x) GLClearError();\
         x;\
ASSERT(GLLogCall(#x, __FILE__, __LINE__));

static void GLClearError()
{
    while(glGetError() != GL_NO_ERROR);
};

static bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << ") " << function << file << ":" << line << std::endl;
        return false;
    }
    return true;
};

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);
    
    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };
    
    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    
    while (getline(stream,line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                //                set mode to vertex
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                //                set mode to frag,emt
                type = ShaderType::FRAGMENT;
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }
    return {ss[0].str(), ss[1].str()};
};

static unsigned int CompileShader(unsigned int type,const std::string&source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id,1,&src,nullptr);
    glCompileShader(id);

    // TODO: Error Handling
    int result;
    glGetShaderiv(id,GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE){
        int lenght;
        glGetShaderiv(id,GL_INFO_LOG_LENGTH,&lenght);
//        char message[lenght];
        char* message = (char*)alloca(lenght * sizeof(char));
        glGetShaderInfoLog(id,lenght,&lenght, message);
        std::cout << "Failed to compile" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
};

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program,vs);
    glAttachShader(program,fs);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
};

float zoom = 1.0f; // Initial zoom level
double offsetX = 0.0f; // Initial X-offset
double offsetY = 0.0f; // Initial Y-offset

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_UP) {
            // Zoom in (reduce zoom value)
            zoom -= 500.0f;
        } else if (key == GLFW_KEY_DOWN) {
            // Zoom out (increase zoom value)
            zoom += 500.0f;
        } else if (key == GLFW_KEY_LEFT) {
            // Move left
            offsetX -= 0.01f;
        } else if (key == GLFW_KEY_RIGHT) {
            // Move right
            offsetX += 0.01f;
        } else if (key == GLFW_KEY_PAGE_UP) {
            // Move up
            offsetY += 0.01f;
        } else if (key == GLFW_KEY_PAGE_DOWN) {
            // Move down
            offsetY -= 0.01f;
        }
    }
}


int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

//  TODO: Understand why this is necessary
#ifdef __APPLE__
    std::cout << "I'm apple machine" << std::endl;
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1000, 1000, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    unsigned int major = glfwGetWindowAttrib(window,GLFW_CONTEXT_VERSION_MAJOR);
    unsigned int minor = glfwGetWindowAttrib(window,GLFW_CONTEXT_VERSION_MINOR);
    std::cout << "OPENGL shader version: " << major << "." << minor << std::endl;

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Glue not initialized" << std::endl;
    };

    float positions[] = {

        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f,
        -1.0f,1.0f

    };
    
    unsigned int indices[] =
    {
        0,1,2,
        2,3,0
    };

//    std::cout << glGetString(GL_VERSION) << std::endl; 


    unsigned int buffer, vao, ibo;
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW);


    glEnableVertexAttribArray(0); // Enables the buffer layout
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    
    GLCall(ShaderProgramSource source = ParseShader("../res/shaders/Basic.shaders"));
    
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::cout << "Current Directory: " << currentPath << std::endl;
    std::cout << "source.VertexSource" << std::endl;
    std::cout << source.VertexSource << std::endl;
    std::cout << "source.FragmentSource" << std::endl;
    std::cout << source.FragmentSource << std::endl;
    
    unsigned int shader = CreateShader(source.VertexSource,source.FragmentSource);
    glLinkProgram(shader);
    glUseProgram(shader);

    GLint zoomLocation = glGetUniformLocation(shader, "u_Zoom");
    GLint offsetLocation = glGetUniformLocation(shader, "u_Offset");

    glfwSetKeyCallback(window, key_callback);
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        
        /* Render here */
        
        glClear(GL_COLOR_BUFFER_BIT);
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
        
        glUniform1f(zoomLocation, zoom);
        glUniform2f(offsetLocation, offsetX,offsetY);

//        Legacy
//----------------------------------------------------

//        glBegin(GL_TRIANGLES);
//        glVertex2f(-0.5f, -0.5f);
//        glVertex2f(0.0f, 0.5f);
//        glVertex2f(0.5f, 0.5f);
//        glEnd();

//----------------------------------------------------

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &ibo);
    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}


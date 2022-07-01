#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    // Constructors
    Shader()
    { }
    
    // constructor generates the shader on the fly
    Shader(const char* vertexPath, const char* fragmentPath)
    {    
        _id = create_program(vertexPath, fragmentPath);
    }
    
    // Access
    GLuint id() { return _id; }
    
    // Read a shader source from a file
    // store the shader source in a std::vector<char>
    void
    read_shader_src(const char *fname, std::vector<char> &buffer)
    {
        std::ifstream in;
        in.open(fname, std::ios::binary);

        if(in.is_open()) {
            // Get the number of bytes stored in this file
            in.seekg(0, std::ios::end);
            size_t length = (size_t)in.tellg();

            // Go to start of the file
            in.seekg(0, std::ios::beg);

            // Read the content of the file in a buffer
            buffer.resize(length + 1);
            in.read(&buffer[0], length);
            in.close();
            // Add a valid C - string end
            buffer[length] = '\0';
        } else {
            std::cerr << "Unable to open " << fname << " I'm out!" << std::endl;
            exit(-1);
        }
    }

    // Compile a shader
    GLuint
    load_and_compile_shader(const char *fname, GLenum shaderType)
    {
        // Load a shader from an external file
        std::vector<char> buffer;
        read_shader_src(fname, buffer);
        const char *src = &buffer[0];

        // Compile the shader
        GLuint shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &src, NULL);
        glCompileShader(shader);
        // Check the result of the compilation
        GLint test;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &test);
        if(!test) {
            std::cerr << "Shader compilation failed with this message:" << std::endl;
            std::vector<char> compilation_log(512);
            glGetShaderInfoLog(shader, compilation_log.size(), NULL, &compilation_log[0]);
            std::cerr << &compilation_log[0] << std::endl;
            glfwTerminate();
            exit(-1);
        }
        return shader;
    }

    // Create a program from two shaders
    GLuint
    create_program(const char *path_vert_shader, const char *path_frag_shader)
    {
        // Load and compile the vertex and fragment shaders
        GLuint vertexShader = load_and_compile_shader(path_vert_shader, GL_VERTEX_SHADER);
        GLuint fragmentShader = load_and_compile_shader(path_frag_shader, GL_FRAGMENT_SHADER);

        // Attach the above shader to a program
        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);

        // Link and use the program
        glLinkProgram(shaderProgram);
       
        // delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return shaderProgram;
    }
    
    // activate the shader
    void activate()
    { 
        glUseProgram(_id);
    }


private:
    GLuint _id;
};
#endif

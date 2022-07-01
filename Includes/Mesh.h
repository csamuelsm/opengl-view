#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>

#include <GL/glew.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Material.h>
#include <Shader.h>

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texture coordinates
    glm::vec2 TextureCoords;
};

// triangular face
struct Face {
    glm::uvec3 Index;
};

struct Texture {
    GLuint         id;
    int            width, height;
    unsigned char *data;
};

class Mesh {
public:
    // constructor
    Mesh(std::vector<Vertex> vertices, std::vector<Face> faces,
        Material &material, Texture &texture)
    {
        _material = material;
        _texture  = texture;
        _vertices = vertices;
        _faces    = faces;
        _matrix   = glm::mat4(1.0f);

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        _setup_for_rendering();
    }

    // render the mesh
    void render(Shader &shader, glm::mat4& global)
    {
        // pass material to vertex shader
        glUniform4fv(glGetUniformLocation(shader.id(), "material.ambient"), 1,
            glm::value_ptr(_material.ambient));
        glUniform4fv(glGetUniformLocation(shader.id(), "material.diffuse"), 1,
            glm::value_ptr(_material.diffuse));
        glUniform4fv(glGetUniformLocation(shader.id(), "material.specular"), 1,
            glm::value_ptr(_material.specular));
        glUniform1f(glGetUniformLocation(shader.id(), "material.shininess"),
            _material.shininess);
        
        // concatenate global and local model matrices
        glm::mat4 m = global*_matrix;
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), "model"), 1, GL_FALSE, glm::value_ptr(m));
        
        glBindVertexArray(_vao);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(shader.id(), "fSampler"), 0);
        
        glBindTexture(GL_TEXTURE_2D, _texture.id);
        glDrawElements(GL_TRIANGLES, _faces.size() * sizeof(Face), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void set_material(Material &m)
    {
        _material = m;
    }
    
    void set_matrix(glm::mat4& m)
    {
        _matrix = m;
    }
    
    Vertex& vertex(GLuint i)
    { return _vertices[i]; }
    
    size_t number_of_vertices()
    { return _vertices.size(); }


private:
    // initializes all the buffer objects/arrays
    void _setup_for_rendering()
    {
        // create vertex array object
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);
        
        // create vertices and faces buffers
        glGenBuffers(1, &_vbo);
        glGenBuffers(1, &_ebo);
        
        // load data into vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(Vertex), &_vertices[0], GL_STATIC_DRAW);

        // load data into element buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _faces.size() * sizeof(Face), &_faces[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
        // vertex Normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // Texture Coordinates
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TextureCoords));

        glBindVertexArray(0);
        
        // Setup texture object
        glGenTextures(1, &(_texture.id));
        glBindTexture(GL_TEXTURE_2D, _texture.id);
                
        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _texture.width, _texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _texture.data);
        glGenerateMipmap(GL_TEXTURE_2D);
         
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    // mesh Data
    Material            _material;
    Texture             _texture;
    glm::mat4           _matrix;
    std::vector<Vertex> _vertices;
    std::vector<Face>   _faces;

    // render data
    GLuint _vao;
    GLuint _vbo, _ebo;
};
#endif

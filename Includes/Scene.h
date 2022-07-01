#ifndef SCENE_H
#define SCENE_H

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Projection.h>
#include <View.h>
#include <Light.h>
#include <Shader.h>
#include <Model.h>

class Scene
{
public:
    Scene()
    { _width = 400; _height = 400; }

    Scene(GLuint w, GLuint h)
        : _width(w), _height(h)
    { }

    void set_projection(GLfloat fov, GLfloat aspect, GLfloat near, GLfloat far)
    { _projection = Projection(fov, aspect, near, far); }
    
    void set_projection(GLfloat xmin, GLfloat xmax, GLfloat ymin, GLfloat ymax,
                        GLfloat zmin, GLfloat zmax)
    { _projection = Projection(xmin, xmax, ymin, ymax, zmin, zmax); }

    void set_view(glm::vec3 eye, glm::vec3 at, glm::vec3 up)
    { _view = View(eye,at,up); }

    void set_light(glm::vec3 position,
                  glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular)
    { _light = Light {position, ambient, diffuse, specular}; }
    
    void set_light(Light light)
    { _light = light; }

    void set_shader(const char* vspath, const char* fspath)
    { _shader = Shader(vspath, fspath); }
    
    void render()
    {      
        glUniformMatrix4fv(glGetUniformLocation(_shader.id(), "view"), 1, GL_FALSE, glm::value_ptr(_view.get_matrix()));
        glUniformMatrix4fv(glGetUniformLocation(_shader.id(), "projection"), 1, GL_FALSE, glm::value_ptr(_projection.get_matrix()));
        
        // pass light to vertex shader
        glUniform3fv(glGetUniformLocation(_shader.id(), "light.position"), 1,
            glm::value_ptr(_light.position));
        glUniform4fv(glGetUniformLocation(_shader.id(), "light.ambient"), 1,
            glm::value_ptr(_light.ambient));
        glUniform4fv(glGetUniformLocation(_shader.id(), "light.diffuse"), 1,
            glm::value_ptr(_light.diffuse));
        glUniform4fv(glGetUniformLocation(_shader.id(), "light.specular"), 1,
            glm::value_ptr(_light.specular));

        _shader.activate();
        for (int i = 0; i < _model.size(); ++i) {
            _model[i].render(_shader);
        }
    }
    
    void add_model(std::vector<Vertex> vertices, std::vector<Face> faces,
        Material &material, Texture &texture)
    {
        _model.push_back(Model(vertices, faces, material, texture));
    }
    
    void add_model(const char *path)
    {
        _model.push_back(Model(path));
    }

    void add_model(Model model)
    {
        _model.push_back(model);
    }
    
    size_t number_of_models()
    { return _model.size(); }
    
    Model& model(unsigned int i)
    {
        return _model[i];
    }


private:
    GLuint             _width, _height;
    Projection         _projection;
    View               _view;
    Light              _light;
    Shader             _shader;
    std::vector<Model> _model;
};
#endif // SCENE_H


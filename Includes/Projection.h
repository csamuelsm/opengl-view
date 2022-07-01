#ifndef PROJECTION_H
#define PROJECTION_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

class Projection
{
public:

    enum Projection_type {
        PROJECTION_UNDEFINED    = -1,
        PROJECTION_ORTHOGRAPHIC =  0,
        PROJECTION_PERSPECTIVE =  1,
    };
        
    Projection()
    { _type = Projection_type::PROJECTION_UNDEFINED; }

    Projection(GLfloat fov, GLfloat aspect, GLfloat near, GLfloat far)
    {
        _matrix = glm::perspective(fov, aspect, near, far);
        _type   = Projection_type::PROJECTION_PERSPECTIVE;
    }    

    // constructor with vectors
    Projection(GLfloat xmin, GLfloat xmax, GLfloat ymin, GLfloat ymax,
               GLfloat zmin, GLfloat zmax)
    {
        _matrix = glm::ortho(xmin, xmax, ymin, ymax, zmin, zmax);
        _type   = Projection_type::PROJECTION_ORTHOGRAPHIC;
    }
    
    // Accesss
    glm::mat4 get_matrix()
    {
        return _matrix;
    }

private:
    // Projection Attributes
    Projection_type _type;
    glm::mat4       _matrix;
};
#endif // PROJECTION_H

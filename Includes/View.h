#ifndef VIEW_H
#define VIEW_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

class View
{
public:
    // constructors/destructors
    View()
    { _matrix = glm::mat4(1.0f); }
    
    View(glm::vec3 eye, glm::vec3 at, glm::vec3 up)
       : _eye(eye), _at(at), _up(up)
    { _matrix = glm::lookAt(_eye,_at,_up); }
    
    // returns the view matrix
    glm::mat4 get_matrix()
    { return _matrix; }
    
    // returns the view matrix
    glm::vec3 get_position()
    { return _eye; }

private:
    // View Attributes
    glm::vec3 _eye;
    glm::vec3 _at;
    glm::vec3 _up;
    glm::mat4 _matrix;
};
#endif // VIEW_H

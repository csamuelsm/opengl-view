#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

struct Light {
    glm::vec3 position;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
};

#endif // LIGHT_H

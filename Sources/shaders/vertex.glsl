#version 330

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 vNormal;
layout(location = 2) in vec2 vTexCoord;

out vec3 fN;
out vec3 fL;
out vec3 fE;
out vec2 texCoord;

struct Light {
  vec3 position;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
};

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform Light light;

void main()
{
    mat4 ModelView = view * model;

    fN = transpose(inverse(mat3(model))) * vNormal.xyz;
    fE = vPosition.xyz;
    fL = light.position;
    
    texCoord    = vTexCoord;
    
    gl_Position = projection * ModelView * vPosition;
}

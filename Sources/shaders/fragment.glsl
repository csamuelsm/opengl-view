#version 330

in vec3 fN;
in vec3 fL;
in vec3 fE;
in vec2 texCoord;

struct Light {
  vec3 position;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
};

struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};

uniform mat4 model;
uniform mat4 view;

uniform Light    light;
uniform Material material;

uniform sampler2D fSampler;

void main()
{   
    vec3 N = normalize(fN);
    vec3 L = normalize(fL);
    vec3 E = normalize(fE);
    
    vec3 H = normalize(L + E);
    
    // compute terms from Blinn-Phong illumination model
    vec4 ambient = material.ambient * light.ambient;
    
    //vec4 diffuse = material.diffuse * light.diffuse * max(dot(L, N), 0.0);
    vec4 diffuse = material.diffuse * texture2D(fSampler, texCoord);
    
    float value = max(dot(N, H), 0.0);
    vec4 specular = material.specular * light.specular * pow(value, material.shininess);
    if (dot(L, N) < 0.0)
        specular = vec4(0.0, 0.0, 0.0, 1.0);
      
    gl_FragColor = vec4((ambient + diffuse + specular).xyz, 1.0);
}

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <cassert>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Mesh.h>
#include <Shader.h>
#include <Scene.h>
#include <FastNoiseLite.h>

#define UP_DIRECTION 100
#define DOWN_DIRECTION 010

#define WATER 1
#define BEACH 2
#define FOREST 3
#define JUNGLE 4
#define SAVANNAH 5
#define DESERT 6
#define SNOW 7

std::string program_name;
GLsizei width, height; // window size

float theta = 0;
float increment = 10.0*M_PI/180.0;
float alpha = 60.0 * M_PI/180.0;

int noMove = 0;
int idleTime = 300;
bool idle = false;

float head_theta = 0;
float head_increment = 1.0*M_PI/180.0;
float head_alpha = 45.0 * M_PI/180.0;

glm::vec3 last;
bool ballEnabled = false;

int direction = UP_DIRECTION;

// camera
glm::vec3 eye(6.0,5.0,6.0);
glm::vec3 at(0.0,0.0,-1.0);
glm::vec3 up(0.0,1.0,0.0);

//glm::mat4 view;

class Terrain
{
public:
  float* generate(int x_dim, int y_dim)
  {
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    float * noiseData = new float[x_dim * y_dim];
    int index = 0;

    for (int y = 0; y < y_dim; y++) {
      for (int x = 0; x < x_dim; x++) {
        double nx = x/5.0 + 0.5;
        double ny = y/5.0 + 0.5;
        //std::cout << nx << ", " << ny << std::endl;
        float e = 1 * noise.GetNoise((float)(1 * nx), (float)(1 * ny)) +  0.5 * noise.GetNoise((float)(2 * nx), (float)(2 * ny)) + 0.25 * noise.GetNoise((float)(4 * nx), (float)(4 * ny));
        e = e / (1.0 + 0.5 + 0.25);
        noiseData[index++] = e * 32.0 - 6.0;
      }
    }

    return noiseData;
  }

  int * getBiome(float * noiseData, int x_dim, int y_dim) {
    int * biomeData = new int[x_dim * y_dim];
    int index = 0;

    for (int y = 0; y < y_dim; y++) {
      for (int x = 0; x < x_dim; x++) {
        float e = (noiseData[index++] + 6.0)/32.0;
        if (e < 0.1) biomeData[index] = WATER;
        else if (e < 0.2) biomeData[index] = BEACH;
        else if (e < 0.3) biomeData[index] = FOREST;
        else if (e < 0.5) biomeData[index] = JUNGLE;
        else if (e < 0.7) biomeData[index] = SAVANNAH;
        else if (e < 0.9) biomeData[index] = DESERT;
        else biomeData[index] = SNOW;
      }
    }

    return biomeData;
  }
};

class MyScene : public Scene
{
public:
    void walk()
    {
        noMove = 0;

        theta += increment;
        if (theta >= 2*M_PI) {
          theta = 0;
        }
        // perna esquerda
        glm::vec3 legL_RotationAxis = leg_top_center(2);
        // we know meshes #2 and #5 are left and right legs, respectively
        glm::mat4 matrix = glm::translate(glm::mat4(1.0f), legL_RotationAxis);
        matrix = glm::rotate(matrix, alpha*sin(theta), glm::vec3(0.0,0.0,1.0));
        matrix = glm::translate(matrix, -legL_RotationAxis);

        model(0).mesh(2).set_matrix(matrix);

        // perna direita
        glm::vec3 legR_RotationAxis = leg_top_center(5);
        // we know meshes #2 and #5 are left and right legs, respectively
        glm::mat4 matrixR = glm::translate(glm::mat4(1.0f), legR_RotationAxis);
        matrixR = glm::rotate(matrixR, -alpha*sin(theta), glm::vec3(0.0,0.0,1.0));
        matrixR = glm::translate(matrixR, -legR_RotationAxis);

        model(0).mesh(5).set_matrix(matrixR);

        // braço esquerdo
        glm::vec3 armL_RotationAxis = leg_top_center(3);
        // we know meshes #2 and #5 are left and right legs, respectively
        glm::mat4 matrixArmL = glm::translate(glm::mat4(1.0f), armL_RotationAxis);
        matrixArmL = glm::rotate(matrixArmL, -alpha*sin(theta), glm::vec3(0.0,0.0,1.0));
        matrixArmL = glm::translate(matrixArmL, -armL_RotationAxis);

        model(0).mesh(3).set_matrix(matrixArmL);

        // braço direito
        glm::vec3 armR_RotationAxis = leg_top_center(4);
        // we know meshes #2 and #5 are left and right legs, respectively
        glm::mat4 matrixArmR = glm::translate(glm::mat4(1.0f), armR_RotationAxis);
        matrixArmR = glm::rotate(matrixArmR, alpha*sin(theta), glm::vec3(0.0,0.0,1.0));
        matrixArmR = glm::translate(matrixArmR, -armR_RotationAxis);

        model(0).mesh(4).set_matrix(matrixArmR);

    }

    void idle()
    {
      head_theta += head_increment;
      if (head_theta >= 2*M_PI) {
        head_theta = 0;
      }

      glm::vec3 head_RotationAxis = leg_top_center(1);
      glm::mat4 matrix = glm::translate(glm::mat4(1.0f), head_RotationAxis);
      matrix = glm::rotate(matrix, head_alpha*sin(head_theta), glm::vec3(0.0,1.0,0.0));
      matrix = glm::translate(matrix, -head_RotationAxis);

      model(0).mesh(0).set_matrix(matrix);
    }

    void move_steve()
    {
      glm::mat4 matrix = glm::translate(model(0).matrix(), glm::vec3(0.25,0.0,0.0));
      model(0).set_matrix(matrix);
    }

    void move_steve_vertical(int direction)
    {
      glm::mat4 matrix;
      if (direction == UP_DIRECTION) matrix = glm::translate(model(0).matrix(), glm::vec3(0.0,0.25,0.0));
      else matrix = glm::translate(model(0).matrix(), glm::vec3(0.0,-0.25,0.0));
      model(0).set_matrix(matrix);
    }

    void rotate_steve(bool clockwise, float angle)
    {
      glm::vec3 body_RotationAxis = leg_top_center(0);
      glm::mat4 matrix = glm::translate(model(0).matrix(), body_RotationAxis);
      float rotationAngle = angle * (M_PI/180.0);
      if (clockwise)
        matrix = glm::rotate(matrix, rotationAngle, glm::vec3(0.0, 1.0, 0.0));
      else
        matrix = glm::rotate(matrix, -rotationAngle, glm::vec3(0.0, 1.0, 0.0));
      matrix = glm::translate(matrix, -body_RotationAxis);
      model(0).set_matrix(matrix);
    }
    
    glm::vec3 leg_top_center(int leg)
    {
       // compute bounding box
       GLfloat xmin(0), xmax(0), ymin(0), ymax(0), zmin(0), zmax(0);
       
       Mesh& mesh = model(0).mesh(leg);
       for (size_t i = 0; i < mesh.number_of_vertices(); ++i) {
           glm::vec3 p = mesh.vertex(i).Position;
           
           std::cout << glm::to_string(p) << std::endl;
           
           xmax = std::max(xmax, p.x);
           xmin = std::min(xmin, p.x);
           ymax = std::max(ymax, p.y);
           ymin = std::min(ymin, p.y);
           zmax = std::max(zmax, p.z);
           zmin = std::min(zmin, p.z);
       }
       
       return glm::vec3(0.5*(xmin + xmax),
                        ymax,
                        0.5*(zmin + zmax));
    }
};

MyScene scene;

// Called when the window is resized
void
window_resized(GLFWwindow *window, int width, int height);

// Called for keyboard events
void
keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);

void
mouse(GLFWwindow* window, int key, int action, int mods);

// Render scene
void
display(GLFWwindow* window);

// Initialize the data to be rendered
void
initialize();

static void
error(int id, const char* description);

int
main(int argc, char *argv[])
{
  GLFWwindow* window;
  program_name = std::string(argv[0]);
  
  // Initialize the library
  if (!glfwInit())
    return EXIT_FAILURE;
    
  // Use OpenGL 3.2 core profile
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwSetErrorCallback(&error);

  // Create a windowed mode window and its OpenGL context
  width = 400;
  height = 400;
  window = glfwCreateWindow(width, height, "OpenGL Viewer", NULL, NULL);
  if (!window) {
    error(-1, "Failed to open a window.");
    glfwTerminate();
    return EXIT_FAILURE;
  }
    
  // Register a callback function for window resize events
  glfwSetWindowSizeCallback(window, &window_resized);

  // Register a callback function for mouse pressed events
  glfwSetMouseButtonCallback(window, &mouse);
  
  // Register a callback function for keyboard pressed events
  glfwSetKeyCallback(window, &keyboard);

  // Make the window's context current
  glfwMakeContextCurrent(window);
  
  // Print the OpenGL version
  std::cout << "OpenGL - " << glGetString(GL_VERSION) << std::endl;
  
  // Initialize GLEW
  glewExperimental = GL_TRUE;
  if(glewInit() != GLEW_OK) {
    error(EXIT_FAILURE, "Failed to initialize GLEW! I'm out!");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // Initialize the scene to be rendered
  initialize();
  
  // Loop until the user closes the window
  while (!glfwWindowShouldClose(window))
  {
    // Display scene
    display(window);

    // Poll for and process events
    glfwPollEvents();
  }
  
  // Terminate GLFW
  glfwTerminate();
  
  return EXIT_SUCCESS;
}

// Render scene
void
display(GLFWwindow* window)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    scene.render();

    // idle movement
    noMove++;
    noMove >= idleTime ? idle = true : idle = false;
    //std::cout << noMove << std::endl;
    if (idle) scene.idle();

    // camera movement
    if (ballEnabled) {
      double curr_x = 0, curr_y = 0;

      glfwGetCursorPos(window, &curr_x, &curr_y);

      double dx = curr_x - last.x;
      double dy = last.y - curr_y;

      float scale_x = abs(dx) / 1000.0;
      float scale_y = abs(dy) / 1000.0;
      float rotSpeed = 45.0f;

      if (dx < 0) {
        // rotação no sentido negativo no eixo Y
        glm::mat4 rotationMat(1);
        rotationMat = glm::rotate(rotationMat, (float)glm::radians(-rotSpeed*scale_x), glm::vec3(0.0, 1.0, 0.0));
        eye = glm::vec3(rotationMat * glm::vec4(eye, 1.0));
        at = glm::vec3(rotationMat * glm::vec4(at, 1.0));
        up = glm::vec3(rotationMat * glm::vec4(up, 1.0));
      } else if (dx > 0) {
        glm::mat4 rotationMat(1);
        rotationMat = glm::rotate(rotationMat, (float)glm::radians(rotSpeed*scale_x), glm::vec3(0.0, 1.0, 0.0));
        eye = glm::vec3(rotationMat * glm::vec4(eye, 1.0));
        at = glm::vec3(rotationMat * glm::vec4(at, 1.0));
        up = glm::vec3(rotationMat * glm::vec4(up, 1.0));
      }

      float rot = rotSpeed * scale_y;
      if (dy < 0) {
        glm::mat4 rotationMat(1);
        rotationMat = glm::rotate(rotationMat, (float)glm::radians(rot), glm::vec3(0.0, 0.0, 1.0));
        eye = glm::vec3(rotationMat * glm::vec4(eye, 1.0));
        at = glm::vec3(rotationMat * glm::vec4(at, 1.0));
        up = glm::vec3(rotationMat * glm::vec4(up, 1.0));
      } else if (dy > 0) {
        glm::mat4 rotationMat(1);
        rotationMat = glm::rotate(rotationMat, (float)glm::radians(-rot), glm::vec3(0.0, 0.0, 1.0));
        eye = glm::vec3(rotationMat * glm::vec4(eye, 1.0));
        at = glm::vec3(rotationMat * glm::vec4(at, 1.0));
        up = glm::vec3(rotationMat * glm::vec4(up, 1.0));
      }
    
      scene.set_view(eye, at, up);
    }

    // Swap front and back buffers
    glfwSwapBuffers(window);
}

void
initialize()
{
  glEnable(GL_DEPTH_TEST);

  // set projection
  scene.set_projection(45.0, (float)width/(float)height, 1.0, 100.0);

  // set view
  //view = glm::lookAt(eye, at, up);
  scene.set_view(eye, at, up);

  // add model from OBJ    
  scene.add_model("Data/Steve.obj");
  // set model matrix
  glm::mat4 matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -4.0f, 0.0f));
  scene.model(0).set_matrix(matrix);

  // terrain
  Terrain terrain;
  int x_dim = 64;
  int y_dim = 64;
  float * noiseData = terrain.generate(x_dim, y_dim);
  //int * biomeData = terrain.getBiome(noiseData, x_dim, y_dim);
  int floor_model = 1;
  int index = 0;

  scene.add_model("Data/Grass_Block.obj");
  Model copy = scene.model(floor_model);

  for (int y = 0; y < y_dim; y++) {
    for (int x = 1; x < x_dim; x++) {
      //std::cout << "(" << x << ", " << y << ") = " << ceil(noiseData[index++]) << std::endl;
      scene.add_model(copy);
    }
  }

  index = 0;

  for (int y = 0; y < y_dim; y++) {
    for (int x = 0; x < x_dim; x++) {
      //std::cout << "(" << x << ", " << y << ") = " << ceil(noiseData[floor_model] * 128.0) << std::endl;
      //(float)ceil(noiseData[floor_model] * 5.0)
      glm::mat4 floor_matrix = glm::translate(glm::mat4(1.0f), glm::vec3((float)x*2.0, ceil(noiseData[index++])*2.0, (float)y*2.0));
      scene.model(floor_model).set_matrix(floor_matrix);
      floor_model++;
    }
  }
  
  std::cout << "Number of models: " << scene.number_of_models() << std::endl;
  
  // set scene light
  Light light = {
    glm::vec3(1.2f, 1.0f, 2.0f), // position
    glm::vec4(0.3f, 0.3f, 0.3f, 1.0f), // ambient
    glm::vec4(0.7f, 0.7f, 0.7f, 1.0f), // diffuse
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), // specular
  };

  // set camera

  eye = scene.leg_top_center(1) - glm::vec3(0.0f, 0.0f, 15.0f);
  glm::mat4 rotationMat = scene.model(0).matrix();
  //glm::mat4 rotationMat(1);
  rotationMat = glm::rotate(rotationMat, (float)glm::radians(90.0), glm::vec3(0.0, 1.0, 0.0));
  eye = glm::vec3(rotationMat * glm::vec4(eye, 1.0));
  at = scene.leg_top_center(1);
  at = glm::vec3(rotationMat * glm::vec4(at, 1.0));
  scene.set_view(eye, at, up);
  
  scene.set_light(light);

  scene.set_shader("Sources/shaders/vertex.glsl", "Sources/shaders/fragment.glsl");
}

// Called when the window is resized
void
window_resized(GLFWwindow* window, int width, int height)
{
  // Use  black to clear the screen
  glClearColor(0, 0, 0, 1);

  // Set the viewport
  glViewport(0, 0, width, height);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glfwSwapBuffers(window);
}

// Called for keyboard events
void
keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  /*switch (key) {
    case GLFW_KEY_W:
    case GLFW_KEY_UP:
      if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (direction == DOWN_DIRECTION) {
            direction = UP_DIRECTION; 
            scene.rotate_steve(false, 180.0);
        }
        scene.walk(); 
        scene.move_steve();       
      }
      break;
    case GLFW_KEY_S:
    case GLFW_KEY_DOWN:
      if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (direction == UP_DIRECTION) {
            direction = DOWN_DIRECTION; 
            scene.rotate_steve(false, 180.0);
        }
        scene.walk(); 
        scene.move_steve();              
      }
      break;
    case GLFW_KEY_Q:
      if (action == GLFW_PRESS) {
        glfwTerminate();
        exit(0);
      }
      break;
    case GLFW_KEY_A:
    case GLFW_KEY_LEFT:
      if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        scene.rotate_steve(false, 20.0);     
      }
      break;
    case GLFW_KEY_D:
    case GLFW_KEY_RIGHT:
      if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        scene.rotate_steve(true, 20.0);     
      }
      break;
    default:
      break;
  }*/

  bool steve_moved = false;

  if (key == GLFW_KEY_W || key == GLFW_KEY_UP){
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
      if (direction == DOWN_DIRECTION) {
          direction = UP_DIRECTION; 
          scene.rotate_steve(false, 180.0);
      }
      scene.walk(); 
      scene.move_steve(); 

      steve_moved = true;
    }
  }
  if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN){
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
      if (direction == UP_DIRECTION) {
          direction = DOWN_DIRECTION; 
          scene.rotate_steve(false, 180.0);
      }
      scene.walk(); 
      scene.move_steve();   

      // move camera together with steve
      glm::mat4 translationMat(1);
      translationMat = glm::translate(translationMat, glm::vec3(-0.25,0.0,0.0));

      eye = glm::vec3(translationMat * glm::vec4(eye, 1.0));
      at = glm::vec3(translationMat * glm::vec4(at, 1.0));
      //up = glm::vec3(translationMat * glm::vec4(up, 1.0));

      scene.set_view(eye, at, up); 

      steve_moved = true;
    }
  }
  if (key == GLFW_KEY_Q){
    if (action == GLFW_PRESS) {
      glfwTerminate();
      exit(0);
    }
  }
  if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT){
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
      scene.rotate_steve(false, 20.0);  
      steve_moved = true;   
    }
  }
  if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT){
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
      scene.rotate_steve(true, 20.0);   
      steve_moved = true;  
    }
  }
  if (key == GLFW_KEY_LEFT_SHIFT){
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
      scene.move_steve_vertical(UP_DIRECTION);
      steve_moved = true;  
    }
  }
  if (key == GLFW_KEY_LEFT_CONTROL){
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
      scene.move_steve_vertical(DOWN_DIRECTION);
      steve_moved = true;  
    }
  }

  // camera update
  if (steve_moved)
  {
    eye = scene.leg_top_center(1) - glm::vec3(0.0f, 0.0f, 15.0f);
    glm::mat4 rotationMat = scene.model(0).matrix();
    //glm::mat4 rotationMat(1);
    rotationMat = glm::rotate(rotationMat, (float)glm::radians(90.0), glm::vec3(0.0, 1.0, 0.0));
    eye = glm::vec3(rotationMat * glm::vec4(eye, 1.0));
    at = scene.leg_top_center(1);
    at = glm::vec3(rotationMat * glm::vec4(at, 1.0));
    scene.set_view(eye, at, up);
    steve_moved = false;
  }
}

void mouse(GLFWwindow* window, int key, int action, int mods){
  switch(key) {
    case GLFW_MOUSE_BUTTON_LEFT:
      //std::cout<< "Mouse button left key" << std::endl;
      if(action == GLFW_PRESS) {
        //std::cout<< "Mouse pressed" << std::endl;
        double curr_x = 0, curr_y = 0;
        glfwGetCursorPos(window, &curr_x, &curr_y);

        last = glm::vec3(curr_x, curr_y, -1);

        ballEnabled = true;
      }

      if(action == GLFW_RELEASE) {
        //std::cout<< "Mouse released" << std::endl;
        ballEnabled = false;
      }

      break;
      

    default:
      break;
  }
}

static void
error(int id, const char* description)
{
  std::cerr << program_name << ": " << description << std::endl;
}


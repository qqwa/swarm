#pragma once

#include "debug_defines.h"
#include "util.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "camera.h"
#include "swarm.h"
#include "wind.h"

class World {
  private:
    glm::vec3 clear_color;

    glm::mat4 view;
    glm::mat4 projection;

    GLFWwindow *m_window;
    Swarm m_swarm;
    Camera m_camera;
    float m_move_speed;
    float m_rotation_speed;

    Wind m_wind;

  public:
    World(GLFWwindow *window);
    void update(float delta);
    void render();
};

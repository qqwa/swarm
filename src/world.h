#pragma once

#include "util.h"
#include <CL/cl.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "camera.h"
#include "enemy.h"
#include "gravitation.h"
#include "swarm.h"
#include "track_point.h"
#include "wind.h"

class World {
  private:
    glm::vec3 clear_color;

    glm::mat4 view;
    glm::mat4 projection;

    cl::Device *m_device;
    cl::Context *m_context;
    cl::CommandQueue *m_queue;

    GLFWwindow *m_window;
    Swarm m_swarm;
    Camera m_camera;
    float m_move_speed;
    float m_rotation_speed;

    Wind m_wind;
    Gravitation m_gravitation;
    TrackPoint m_track_point;
    Enemy m_enemy;

    int tick;

    bool pressed_t;

  public:
    World(GLFWwindow *window, cl::Device *device, cl::Context *context,
          cl::CommandQueue *queue);
    void update(float delta);
    void render();
};

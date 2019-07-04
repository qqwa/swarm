#include "world.h"
#include "config.h"
#include <GL/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

World::World(GLFWwindow *window, cl::Device *device, cl::Context *context, cl::CommandQueue *queue) {
    m_window = window;
    m_device = device;
    m_context = context;
    m_queue = queue;
    clear_color = {0.2, 0.3, 0.3};

    m_move_speed = 50.0;
    m_rotation_speed = 80;

    m_camera.setProjection(glm::perspective(
        glm::radians(45.0f), (float)config->width / (float)config->height, 0.1f, 10000.0f));
    m_camera.update(0);

    tick = 0;
    m_wind = Wind();
    m_gravitation = Gravitation(+9.81);

    if(!config->debug("use_cpu")) {
        m_swarm.create_kernels_and_buffers(*m_device, *m_context);
    }
    m_swarm.reset(*m_queue);

    pressed_t = glfwGetKey(m_window, GLFW_KEY_T) == GLFW_RELEASE;
}

void World::update(float delta) {
    // move camera
    if (config->debug("control_camera")) {
        if (glfwGetKey(m_window, GLFW_KEY_W)) {
            m_camera.move_forward(m_move_speed * delta);
        }
        if (glfwGetKey(m_window, GLFW_KEY_S)) {
            m_camera.move_forward(-m_move_speed * delta);
        }
        if (glfwGetKey(m_window, GLFW_KEY_A)) {
            m_camera.move_left(m_move_speed * delta);
        }
        if (glfwGetKey(m_window, GLFW_KEY_D)) {
            m_camera.move_left(-m_move_speed * delta);
        }
        // rotate camera
        if (glfwGetKey(m_window, GLFW_KEY_UP)) {
            m_camera.rotateHorizontal(m_rotation_speed * delta);
        }
        if (glfwGetKey(m_window, GLFW_KEY_DOWN)) {
            m_camera.rotateHorizontal(-m_rotation_speed * delta);
        }
        if (glfwGetKey(m_window, GLFW_KEY_LEFT)) {
            m_camera.rotateVertical(-m_rotation_speed * delta);
        }
        if (glfwGetKey(m_window, GLFW_KEY_RIGHT)) {
            m_camera.rotateVertical(m_rotation_speed * delta);
        }
    } else {
        m_camera.update(tick);
    }

    m_wind.update(tick);
    m_track_point.update(tick);

    if (config->debug("manuel_tick")) {
        if (glfwGetKey(m_window, GLFW_KEY_T) == GLFW_PRESS && !pressed_t) {
            // runs tick
            pressed_t = true;
        }
        if (glfwGetKey(m_window, GLFW_KEY_T) == GLFW_RELEASE && pressed_t) {
            pressed_t = false;
            return;
        }
    }
    if(config->debug("use_cpu")) {
        m_swarm.simulate_tick_cpu(tick, m_track_point.get_pos(), m_wind,
                            m_gravitation);
    } else {
        m_swarm.simulate_tick_gpu(tick, m_track_point.get_pos(), m_wind,
                            m_gravitation, *m_queue);
    }
    tick++;
}

void World::render() {
    glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw swarm
    m_swarm.render(m_camera);
    m_wind.render(m_camera);
    m_gravitation.render(m_camera);
    glDisable(GL_DEPTH_TEST);
    m_track_point.render(m_camera);
    glEnable(GL_DEPTH_TEST);
}
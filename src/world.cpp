#include "world.h"
#include "config.h"
#include <GL/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

World::World(GLFWwindow *window) {
    m_window = window;
    clear_color = {0.2, 0.3, 0.3};

    m_move_speed = 50.0;
    m_rotation_speed = 80;

    m_camera.setProjection(glm::perspective(
        glm::radians(45.0f), (float)800 / (float)600, 0.1f, 10000.0f));
    m_camera.update(0);

    m_swarm = Swarm();

    tick = 0;
    m_wind = Wind();
    m_gravitation = Gravitation(+9.81);

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
            m_swarm.simulate_tick(m_track_point.get_pos(), m_wind,
                                  m_gravitation);
            pressed_t = true;
        }
        if (glfwGetKey(m_window, GLFW_KEY_T) == GLFW_RELEASE && pressed_t) {
            pressed_t = false;
        }
    } else {
        m_swarm.simulate_tick(m_track_point.get_pos(), m_wind, m_gravitation);
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
    m_track_point.render(m_camera);
}
#include "world.h"
#include <GL/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

World::World(GLFWwindow *window) : m_swarm(8000) {
    m_window = window;
    clear_color = {0.2, 0.3, 0.3};

    m_move_speed = 50.0;
    m_rotation_speed = 80;

    m_camera.setProjection(glm::perspective(
        glm::radians(45.0f), (float)800 / (float)600, 0.1f, 1000.0f));
    m_camera.move_forward(-200);

    m_swarm.reset_transforms(glm::vec3(0), glm::quat(), glm::vec3(0.5));

    m_wind = Wind();
    m_gravitation = Gravitation(+9.81);
}

void World::update(float delta) {
    // move camera
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

    m_swarm.update(delta);
    m_wind.update(delta);
    m_track_point.update(delta);
    // std::cout << "gravitation: " << glm::to_string(m_gravitation.get_force())
    // << std::endl; std::cout << "wind       : " <<
    // glm::to_string(m_wind.get_force()) << std::endl;
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
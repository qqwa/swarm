#include "world.h"
#include <GL/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

World::World(GLFWwindow *window) : m_swarm(8000) {
    m_window = window;
    clear_color = {0.2, 0.3, 0.3};
    bird_color = {0.0, 0.0, 0.0};
    bird = util::loadMesh("res/bird.obj");
    bird_shader = util::getShader("res/shader/bird");

    m_move_speed = 50.0;
    m_rotation_speed = 80;

    m_camera.setProjection(glm::perspective(
        glm::radians(45.0f), (float)800 / (float)600, 0.1f, 1000.0f));
    m_camera.move_forward(-200);

    m_swarm.reset_transforms(glm::vec3(0), glm::quat(), glm::vec3(0.5));
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
}

void World::render() {
    glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw swarm
    glUseProgram(bird_shader);
    int viewLocation = glGetUniformLocation(bird_shader, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE,
                       glm::value_ptr(m_camera.GetTransform()));
    int projectionLocation = glGetUniformLocation(bird_shader, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE,
                       glm::value_ptr(m_camera.GetProjection()));
    m_swarm.render(bird, bird_shader);
}
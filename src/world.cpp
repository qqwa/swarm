#include "world.h"
#include <GL/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

World::World() : m_swarm(8000) {
    clear_color = {0.2, 0.3, 0.3};
    bird_color = {0.0, 0.0, 0.0};
    bird = util::loadMesh("res/bird.obj");
    bird_shader = util::getShader("res/shader/bird");

    view = glm::mat4(1.0f);
    projection = glm::mat4(1.0f);

    view = glm::translate(view, glm::vec3(0.0f, -20.0f, -300.0f));
    view = glm::rotate(view, glm::radians(10.f), glm::vec3(1.f, 0.f, 0.f));
    projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)600,
                                  0.1f, 1000.0f);

    m_swarm.reset_transforms(glm::vec3(0), glm::quat(), glm::vec3(0.5));
}

void World::update(float delta) { m_swarm.update(delta); }

void World::render() {
    glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw swarm
    glUseProgram(bird_shader);
    int viewLocation = glGetUniformLocation(bird_shader, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    int projectionLocation = glGetUniformLocation(bird_shader, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE,
                       glm::value_ptr(projection));
    m_swarm.render(bird, bird_shader);
}
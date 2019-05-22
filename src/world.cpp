#include "world.h"
#include <GL/gl.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

World::World() {
    clear_color = {0.2, 0.3, 0.3};
    bird_color = {0.0, 0.0, 0.0};
    bird = util::loadMesh("res/bird.obj");
    bird_shader = util::getShader("res/shader/bird");

    view = glm::mat4(1.0f);
    projection = glm::mat4(1.0f);
    model = glm::mat4(1.0f);

    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -15.0f));
    projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);
    
}

void World::update(float delta) {
    model = glm::rotate(model, glm::radians(10.0f * delta), glm::vec3(1.0f, 0.3f, 0.5f));
}

void World::render() {
        glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0);
        glEnable(GL_DEPTH_TEST);  
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw one bird mesh at 0, 0, 0
        glUseProgram(bird_shader);

        int viewLocation = glGetUniformLocation(bird_shader, "view");
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        
        int projectionLocation = glGetUniformLocation(bird_shader, "projection");
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
        
        int modelLocation = glGetUniformLocation(bird_shader, "model");
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        
        glBindVertexArray(bird.vao);
        glDrawElements(GL_TRIANGLES, bird.indicesCount,GL_UNSIGNED_INT, 0);
}
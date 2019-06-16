#include "swarm.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

Swarm::Swarm(size_t member_count)
    : m_posistions(member_count), m_orientations(member_count),
      m_scales(member_count) {
    bird_color = {0.0, 0.0, 0.0};
    bird_mesh = util::loadMesh("res/bird.obj");
    bird_shader = util::getShader("res/shader/default");
}

void Swarm::reset_transforms(glm::vec3 position, glm::quat orientation,
                             glm::vec3 scale) {
    // arange swarm equally in a cube with edge length size^(1/3), rounded
    // upwards
    int edge_length = (int)ceil(pow(size(), 1.f / 3.f));

    auto distance_factor = 5.0;
    glm::vec3 pos = position - glm::vec3((edge_length / 2.0) * distance_factor);

    for (int i = 0; i < size(); i++) {
        int tmp_index = i;
        int z = tmp_index / (edge_length * edge_length);
        tmp_index %= (edge_length * edge_length);
        int y = tmp_index / edge_length;
        int x = tmp_index % edge_length;

        m_posistions[i] =
            pos + glm::vec3(x * distance_factor, y * distance_factor,
                            z * distance_factor);
        m_orientations[i] = orientation;
        m_scales[i] = scale;
    }
}

size_t Swarm::size() { return m_posistions.size(); }

void Swarm::update(float delta) {
    for (int i = 0; i < size(); i++) {
        m_posistions[i] += glm::vec3(0, 0, 1 * delta);
        m_orientations[i] = glm::rotate(
            m_orientations[i], glm::radians(5 * delta), glm::vec3(1, 0, 0));
        m_orientations[i] = glm::normalize(m_orientations[i]);
    }
}

void Swarm::render(Camera &camera) {

    glUseProgram(bird_shader);
    int viewLocation = glGetUniformLocation(bird_shader, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE,
                       glm::value_ptr(camera.GetTransform()));
    int projectionLocation = glGetUniformLocation(bird_shader, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE,
                       glm::value_ptr(camera.GetProjection()));
    int colorLocation = glGetUniformLocation(bird_shader, "color");
    glUniform3fv(colorLocation, 1, glm::value_ptr(bird_color));

    //    std::cout << "Rendering " << size() << " birds." << std::endl;
    for (int i = 0; i < size(); i++) {
        auto model =
            Transform::Matrix(m_posistions[i], m_orientations[i], m_scales[i]);
        int modelLocation = glGetUniformLocation(bird_shader, "model");
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(bird_mesh.vao);
        glDrawElements(GL_TRIANGLES, bird_mesh.indicesCount, GL_UNSIGNED_INT,
                       0);
    }
}
#include "swarm.h"
#include "config.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <random>

Swarm::Swarm() {
    bird_color = {0.0, 0.0, 0.0};
    bird_mesh = util::loadMesh("res/bird.obj");
    bird_shader = util::getShader("res/shader/default");

    reset();
}

void Swarm::reset() {
    // arange swarm equally in a cube with edge length size^(1/3), rounded
    // upwards

    m_random.seed({config->random_seed});
    auto count = config->swarm_size;

    int radius =
        (int)ceil(pow(count, 1.f / 3.f)) * config->swarm_initial_spread;
    auto gen_coord = std::uniform_real_distribution<float>(-radius, radius);

    // generate all birds randomly inside of a cube
    for (int i = 0; i < count; i++) {
        auto pos = glm::vec3{gen_coord(m_random), gen_coord(m_random),
                             gen_coord(m_random)};
        m_posistions.push_back(config->swarm_start + pos);
        m_orientations.push_back(glm::quat());
        m_scales.push_back({1, 1, 1});
    }

    // all birds get 4 randomly assigned neighbours
    auto gen_neighbour = std::uniform_int_distribution<size_t>(0, count - 1);
    for (int i = 0; i < count; i++) {
        m_neighbors.push_back({gen_neighbour(m_random), gen_neighbour(m_random),
                               gen_neighbour(m_random),
                               gen_neighbour(m_random)});
    }

    // run swarm simulation for 10_000 ticks with track_point position =
    // swarm_start so that neighbours find to another
    update_swarm_center();
    for (int i = 0; i < 10000; i++) {
        simulate_tick(config->swarm_start);
    }
}

size_t Swarm::size() { return m_posistions.size(); }

void Swarm::update_swarm_center() {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    for (int i = 0; i < config->swarm_size; i++) {
        x += (double)m_posistions[i].x / config->swarm_size;
        y += (double)m_posistions[i].y / config->swarm_size;
        z += (double)m_posistions[i].z / config->swarm_size;
    }

    m_swarm_center = {x, y, z};
}

void Swarm::simulate_tick(glm::vec3 track_point) {
    if (config->debug("use_cpu")) {
        simulate_cpu(track_point);
    } else {
        simulate_gpu();
    }
}

// programmed as it were a "kernel"
void Swarm::simulate_cpu(glm::vec3 track_point) {
    glm::vec3 update_swarm_center = {0, 0, 0};
    for (int i = 0; i < config->swarm_size; i++) {
        auto pos = m_posistions[i];

        /////////////////////////////////////////////////////////////////////////////
        // 1. check distance to neighbours and update them
        /////////////////////////////////////////////////////////////////////////////

        // find potential canidates
        size_t new_neighbours[] = {0, 0, 0, 0};
        for (int n = 0; n < 4; i++) { // find best canidate of neighbour 1
            auto neighbour = m_neighbors[i][n];
            auto dist = glm::length(m_posistions[neighbour] - pos);
            auto neighbour1 = m_neighbors[neighbour][0];
            auto neighbour2 = m_neighbors[neighbour][1];
            auto neighbour3 = m_neighbors[neighbour][2];
            auto neighbour4 = m_neighbors[neighbour][3];

            if (glm::length(m_posistions[neighbour1] - pos) < dist) {
                dist = glm::length(m_posistions[neighbour1] - pos);
                new_neighbours[n] = neighbour1;
            }
            if (glm::length(m_posistions[neighbour2] - pos) < dist) {
                dist = glm::length(m_posistions[neighbour2] - pos);
                new_neighbours[n] = neighbour2;
            }
            if (glm::length(m_posistions[neighbour3] - pos) < dist) {
                dist = glm::length(m_posistions[neighbour3] - pos);
                new_neighbours[n] = neighbour3;
            }
            if (glm::length(m_posistions[neighbour4] - pos) < dist) {
                dist = glm::length(m_posistions[neighbour4] - pos);
                new_neighbours[n] = neighbour4;
            }
        }

        // update own neighbours, if it would result in a duplicate dont update
        if (new_neighbours[0] != m_neighbors[i][1] &&
            new_neighbours[0] != m_neighbors[i][2] &&
            new_neighbours[0] != m_neighbors[i][3]) {
            m_neighbors[i][0] = new_neighbours[0];
        }

        if (new_neighbours[1] != m_neighbors[i][0] &&
            new_neighbours[1] != m_neighbors[i][2] &&
            new_neighbours[1] != m_neighbors[i][3]) {
            m_neighbors[i][1] = new_neighbours[1];
        }

        if (new_neighbours[2] != m_neighbors[i][0] &&
            new_neighbours[2] != m_neighbors[i][1] &&
            new_neighbours[2] != m_neighbors[i][3]) {
            m_neighbors[i][2] = new_neighbours[2];
        }

        if (new_neighbours[3] != m_neighbors[i][0] &&
            new_neighbours[3] != m_neighbors[i][1] &&
            new_neighbours[3] != m_neighbors[i][2]) {
            m_neighbors[i][3] = new_neighbours[3];
        }

        /////////////////////////////////////////////////////////////////////////////
        // 2. try to center between neighbors
        /////////////////////////////////////////////////////////////////////////////
        auto center_neighbours_direction = glm::normalize(
            (m_posistions[new_neighbours[0]] + m_posistions[new_neighbours[1]] +
             m_posistions[new_neighbours[2]] +
             m_posistions[new_neighbours[3]]) /
                4.0f -
            pos);

        /////////////////////////////////////////////////////////////////////////////
        // 3. fly in direction of track point
        /////////////////////////////////////////////////////////////////////////////
        auto tp_direction = glm::normalize(track_point - pos);

        /////////////////////////////////////////////////////////////////////////////
        // 4. check if collideable object is near, if yes fly away from it
        /////////////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////////////
        // 5. fly towards swarm center , and limits: acceleration, max speed,
        // max angle change...
        /////////////////////////////////////////////////////////////////////////////
        auto swarm_center_direction = glm::normalize(m_swarm_center - pos);

        /////////////////////////////////////////////////////////////////////////////
        // 6. apply 2-6 relativ to setted ratios
        /////////////////////////////////////////////////////////////////////////////
        auto target_direction = glm::normalize(
            config->swarm_weight_neighbours * center_neighbours_direction +
            config->swarm_weight_track_point * tp_direction +
            config->swarm_weight_swarm_center * swarm_center_direction);

        // TODO: acceleration, realistic direction change, etc..
        auto final_direction = target_direction;
        auto pos_update = final_direction * config->swarm_speed * 0.0166667f;

        m_orientations[i] = {final_direction};
        m_posistions[i] += pos_update;

        /////////////////////////////////////////////////////////////////////////////
        // 7. update swarm center, with change vector of all members
        /////////////////////////////////////////////////////////////////////////////
        update_swarm_center += pos_update;
    }
    m_swarm_center += update_swarm_center;
}

void Swarm::simulate_gpu() {}

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
#include "swarm.h"
#include "config.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <random>

Swarm::Swarm() {
    reset();

    bird_color = {0.0, 0.0, 0.0};
    bird_mesh = util::loadMesh("res/bird.obj");
    if (config->debug("instance_rendering")) {
        bird_shader = util::getShader("res/shader/instance");

        glGenBuffers(1, &instance_pos_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, instance_pos_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * config->swarm_size,
                     m_posistions.data(), GL_DYNAMIC_DRAW);

        glGenBuffers(1, &instance_dir_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, instance_dir_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * config->swarm_size,
                     m_orientations.data(), GL_DYNAMIC_DRAW);

        glBindVertexArray(bird_mesh.vao);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, instance_pos_vbo);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glVertexAttribDivisor(1, 1);

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, instance_dir_vbo);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glVertexAttribDivisor(2, 1);
    } else {
        bird_shader = util::getShader("res/shader/default");
    }

    center_color = {1.0, 0.7, 1.0};
    center_mesh = util::loadMesh("res/sphere.obj");
    center_shader = util::getShader("res/shader/default");
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
        m_orientations.push_back(glm::vec3{1, 1, 1});
        m_scales.push_back({1, 1, 1});
    }

    // all birds get 4 randomly assigned neighbours
    auto gen_neighbour = std::uniform_int_distribution<size_t>(0, count - 1);
    for (int i = 0; i < count; i++) {
        std::vector<size_t> tmp;

        while (tmp.size() != 4) {
            auto number = gen_neighbour(m_random);
            if (number != i) {
                tmp.push_back(number);
            } else {
                std::cout << "tried to set neighbour to self" << std::endl;
            }
        }
        m_neighbors.push_back(tmp);
    }

    // run swarm simulation for 10_000 ticks with track_point position =
    // swarm_start so that neighbours find to another
    update_swarm_center();
    // for (int i = 0; i < 5000; i++) {
    //     simulate_tick(config->swarm_start);
    // }
    // throw std::runtime_error("debug!");
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

void Swarm::simulate_tick(glm::vec3 track_point, Wind wind,
                          Gravitation gravitation) {
    update_neighbours();
    if (config->debug("use_cpu")) {
        simulate_cpu(track_point, wind, gravitation);
    } else {
        simulate_gpu();
    }
}

void Swarm::update_neighbours() {
    config->update_neighbors.Start();
    for (int i = 0; i < config->swarm_size; i++) {
        auto pos = m_posistions[i];
        std::vector<size_t> neigbours = {};
        std::vector<float> distances = {};

        for (auto j = 0; neigbours.size() != 4; j++) {
            if (j != i) {
                neigbours.push_back(j);
                distances.push_back(glm::length(m_posistions[0] - pos));
            }
        }

        int largest = 0;
        for (int j = 1; j < 4; j++) {
            if (distances[largest] < distances[j]) {
                largest = j;
            }
        }

        for (int j = 4; j < config->swarm_size; j++) {
            auto dist = glm::length(m_posistions[j] - pos);
            if (distances[largest] < dist) {
                distances[largest] = dist;
                neigbours[largest] = j;
                int largest = 0;
                for (int j = 1; j < 4; j++) {
                    if (distances[largest] < distances[j]) {
                        largest = j;
                    }
                }
            }
        }
        m_neighbors[i] = neigbours;
    }
    config->update_neighbors.Stop();
}

void Swarm::update_neighbors_incremental() {
    for (int i = 0; i < config->swarm_size; i++) {
    }
}

// programmed as it were a "kernel"
void Swarm::simulate_cpu(glm::vec3 track_point, Wind wind,
                         Gravitation gravitation) {
    config->update_swarm.Start();
    glm::vec3 update_swarm_center = {0, 0, 0};
    // std::cout << "simulate cpu" << std::endl;

    // apply external forces
    for (int i = 0; i < config->swarm_size; i++) {
        auto pos = m_posistions[i];
        pos += wind.get_force() * config->tick;

        m_posistions[i] = pos;
    }
    m_swarm_center += wind.get_force() * config->tick;

    for (int i = 0; i < config->swarm_size; i++) {
        auto pos = m_posistions[i];

        /////////////////////////////////////////////////////////////////////////////
        // 1. try to keep distance x between all neighbours
        /////////////////////////////////////////////////////////////////////////////
        auto neighbour0 = m_posistions[m_neighbors[i][0]] - pos;
        auto neighbour1 = m_posistions[m_neighbors[i][1]] - pos;
        auto neighbour2 = m_posistions[m_neighbors[i][2]] - pos;
        auto neighbour3 = m_posistions[m_neighbors[i][3]] - pos;

        auto dist_neighbour0 = glm::length(neighbour0);
        auto dist_neighbour1 = glm::length(neighbour1);
        auto dist_neighbour2 = glm::length(neighbour2);
        auto dist_neighbour3 = glm::length(neighbour3);

        auto vec_neigbour0 = glm::vec3(0, 0, 0);
        auto vec_neigbour1 = glm::vec3(0, 0, 0);
        auto vec_neigbour2 = glm::vec3(0, 0, 0);
        auto vec_neigbour3 = glm::vec3(0, 0, 0);

        auto neighbour_dist_factor = 5.0;
        auto neighbour_dist_ideal = 100.0;
        auto neighbour_dist_toleration = 25.0;

        if (dist_neighbour0 <
                neighbour_dist_ideal - neighbour_dist_toleration ||
            neighbour_dist_ideal + neighbour_dist_toleration <
                dist_neighbour0) {
            vec_neigbour0 =
                neighbour0 * (float)((dist_neighbour0 - neighbour_dist_ideal) *
                                     neighbour_dist_factor);
        }

        if (dist_neighbour1 <
                neighbour_dist_ideal - neighbour_dist_toleration ||
            neighbour_dist_ideal + neighbour_dist_toleration <
                dist_neighbour1) {
            vec_neigbour1 =
                neighbour1 * (float)((dist_neighbour1 - neighbour_dist_ideal) *
                                     neighbour_dist_factor);
        }

        if (dist_neighbour2 <
                neighbour_dist_ideal - neighbour_dist_toleration ||
            neighbour_dist_ideal + neighbour_dist_toleration <
                dist_neighbour2) {
            vec_neigbour2 =
                neighbour2 * (float)((dist_neighbour2 - neighbour_dist_ideal) *
                                     neighbour_dist_factor);
        }

        if (dist_neighbour3 <
                neighbour_dist_ideal - neighbour_dist_toleration ||
            neighbour_dist_ideal + neighbour_dist_toleration <
                dist_neighbour3) {
            vec_neigbour3 =
                neighbour3 * (float)((dist_neighbour3 - neighbour_dist_ideal) *
                                     neighbour_dist_factor);
        }

        auto neihgour_factor =
            std::max(glm::length(vec_neigbour0 + vec_neigbour1 + vec_neigbour2 +
                                 vec_neigbour3) /
                         4.0f,
                     10.0f);
        auto neighbour_correction = glm::normalize(
            vec_neigbour0 + vec_neigbour1 + vec_neigbour2 + vec_neigbour3);

        /////////////////////////////////////////////////////////////////////////////
        // 2. try to center between neighbors
        /////////////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////////////
        // 3. fly in direction of track point
        /////////////////////////////////////////////////////////////////////////////
        // dont normalize because the length matters?
        auto tp_direction = glm::normalize(track_point - m_swarm_center);

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
        neighbour_correction = glm::normalize(neighbour_correction);
        tp_direction = glm::normalize(tp_direction);
        swarm_center_direction = glm::normalize(swarm_center_direction);

        neighbour_correction *= config->swarm_weight_neighbours;
        tp_direction *= config->swarm_weight_track_point;
        swarm_center_direction *= config->swarm_weight_swarm_center;

        auto target_direction = glm::normalize(
            // 25.0f * spread_direction * glm::length(m_swarm_center - pos) +
            neighbour_correction + tp_direction + swarm_center_direction);

        // TODO: acceleration, realistic direction change, etc..
        auto final_direction =
            m_orientations[i] + target_direction * 0.33f * config->tick;
        final_direction = glm::normalize(final_direction);
        auto pos_update = final_direction * config->swarm_speed * config->tick;

        m_posistions[i] += pos_update;
        m_orientations[i] = {final_direction};

        if (i == 0 && config->debug("print_bird_vectors")) {
            std::cout << "pos: " << glm::to_string(pos) << std::endl
                      << "center: " << glm::to_string(m_swarm_center)
                      << std::endl
                      << "neighbour_correction: "
                      << glm::to_string(neighbour_correction) << std::endl
                      << "  n0 :" << dist_neighbour0 << std::endl
                      << "  n1 :" << dist_neighbour1 << std::endl
                      << "  n2 :" << dist_neighbour2 << std::endl
                      << "  n3 :" << dist_neighbour3 << std::endl
                      << "center_direction:     "
                      << glm::to_string(swarm_center_direction) << std::endl
                      << "tp_direction:         "
                      << glm::to_string(tp_direction) << std::endl
                      << "final_direction       "
                      << glm::to_string(final_direction) << std::endl
                      << "" << std::endl;
        }

        /////////////////////////////////////////////////////////////////////////////
        // 7. update swarm center, with change vector of all members
        /////////////////////////////////////////////////////////////////////////////
        update_swarm_center += pos_update / (float)config->swarm_size;
    }
    m_swarm_center += update_swarm_center;

    config->update_swarm.Stop();
}

void Swarm::simulate_gpu() {
    config->update_swarm.Start();
    config->update_swarm.Stop();
}

void Swarm::render(Camera &camera) {
    // draw birds
    glUseProgram(bird_shader);

    int viewLocation = glGetUniformLocation(bird_shader, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE,
                       glm::value_ptr(camera.GetTransform()));
    int projectionLocation = glGetUniformLocation(bird_shader, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE,
                       glm::value_ptr(camera.GetProjection()));
    int colorLocation = glGetUniformLocation(bird_shader, "color");
    glUniform3fv(colorLocation, 1, glm::value_ptr(bird_color));

    if (config->debug("instance_rendering")) {
        // update instance buffers
        glBindBuffer(GL_ARRAY_BUFFER, instance_pos_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * config->swarm_size,
                     m_posistions.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, instance_dir_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * config->swarm_size,
                     m_orientations.data(), GL_DYNAMIC_DRAW);

        // draw swarm
        glBindVertexArray(bird_mesh.vao);
        glDrawElementsInstanced(GL_TRIANGLES, bird_mesh.indicesCount,
                                GL_UNSIGNED_INT, 0, config->swarm_size);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else {
        for (int i = 0; i < size(); i++) {
            auto model = glm::lookAt(m_posistions[i],
                                     m_posistions[i] - m_orientations[i],
                                     {0.0f, 1.0f, 0.0f});
            model = glm::inverse(model);
            int modelLocation = glGetUniformLocation(bird_shader, "model");
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE,
                               glm::value_ptr(model));
            glBindVertexArray(bird_mesh.vao);
            glDrawElements(GL_TRIANGLES, bird_mesh.indicesCount,
                           GL_UNSIGNED_INT, 0);
        }
    }
    // draw center point

    glUseProgram(center_shader);
    viewLocation = glGetUniformLocation(center_shader, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE,
                       glm::value_ptr(camera.GetTransform()));
    projectionLocation = glGetUniformLocation(center_shader, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE,
                       glm::value_ptr(camera.GetProjection()));
    colorLocation = glGetUniformLocation(center_shader, "color");
    glUniform3fv(colorLocation, 1, glm::value_ptr(center_color));

    auto center =
        Transform(m_swarm_center, glm::vec3{0, 0, 0}, glm::vec3{5, 5, 5});
    int modelLocation = glGetUniformLocation(center_shader, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE,
                       glm::value_ptr(center.GetMatrix()));
    glBindVertexArray(center_mesh.vao);
    glDrawElements(GL_TRIANGLES, center_mesh.indicesCount, GL_UNSIGNED_INT, 0);
}
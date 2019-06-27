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

    center_color = {1.0, 0.7, 1.0};
    center_mesh = util::loadMesh("res/sphere.obj");
    center_shader = util::getShader("res/shader/default");

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

void Swarm::simulate_tick(glm::vec3 track_point) {
    update_neighbours();
    if (config->debug("use_cpu")) {
        simulate_cpu(track_point);
    } else {
        simulate_gpu();
    }
}

void Swarm::update_neighbours() {

}

// programmed as it were a "kernel"
void Swarm::simulate_cpu(glm::vec3 track_point) {
    glm::vec3 update_swarm_center = {0, 0, 0};
    // std::cout << "simulate cpu" << std::endl;
    for (int i = 0; i < config->swarm_size; i++) {
        auto pos = m_posistions[i];

        /////////////////////////////////////////////////////////////////////////////
        // 1. check distance to neighbours and update them
        /////////////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////////////
        // 2. try to center between neighbors
        /////////////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////////////
        // 3. fly in direction of track point
        /////////////////////////////////////////////////////////////////////////////
        // dont normalize because the length matters?
        auto tp_direction = track_point - m_swarm_center;

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
            // 25.0f * spread_direction * glm::length(m_swarm_center - pos) +
            // config->swarm_weight_neighbours * center_neighbours_direction +
            config->swarm_weight_track_point * tp_direction +
            config->swarm_weight_swarm_center * swarm_center_direction);

        // TODO: acceleration, realistic direction change, etc..
        auto final_direction = target_direction;
        auto pos_update = final_direction * config->swarm_speed * 0.0166667f;

        m_posistions[i] += pos_update;
        m_orientations[i] = {final_direction};

        // std::cout << "final_direction " << glm::to_string(final_direction)
        //           << std::endl
        //           << "center_neighbours_direction " <<
        //           glm::to_string(center_neighbours_direction)
        //           << std::endl
        //           << "tp_direction " << glm::to_string(tp_direction)
        //           << std::endl
        //           << "swarm_center_direction " <<
        //           glm::to_string(swarm_center_direction)
        //           << std::endl
        //           << "pos_update " << glm::to_string(pos_update)
        //           << std::endl;

        /////////////////////////////////////////////////////////////////////////////
        // 7. update swarm center, with change vector of all members
        /////////////////////////////////////////////////////////////////////////////
        update_swarm_center += pos_update/(float)config->swarm_size;
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

    for (int i = 0; i < size(); i++) {
        auto model =
            Transform::Matrix(m_posistions[i], m_orientations[i], m_scales[i]);
        int modelLocation = glGetUniformLocation(bird_shader, "model");
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(bird_mesh.vao);
        glDrawElements(GL_TRIANGLES, bird_mesh.indicesCount, GL_UNSIGNED_INT,
                       0);
    }

    // draw center point

    glUseProgram(center_shader);
    viewLocation = glGetUniformLocation(center_shader, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE,
                       glm::value_ptr(camera.GetTransform()));
    projectionLocation =
        glGetUniformLocation(center_shader, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE,
                       glm::value_ptr(camera.GetProjection()));
    colorLocation = glGetUniformLocation(center_shader, "color");
    glUniform3fv(colorLocation, 1, glm::value_ptr(center_color));

    auto center = Transform(m_swarm_center, glm::vec3{0,0,0}, glm::vec3{5, 5, 5});
    int modelLocation = glGetUniformLocation(center_shader, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE,
                       glm::value_ptr(center.GetMatrix()));
    glBindVertexArray(center_mesh.vao);
    glDrawElements(GL_TRIANGLES, center_mesh.indicesCount, GL_UNSIGNED_INT,
                   0);
}
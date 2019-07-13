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
    if (config->debug("instance_rendering")) {
        bird_shader = util::getShader("res/shader/instance");

        glGenBuffers(1, &instance_pos_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, instance_pos_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * config->swarm_size,
                     m_positions.data(), GL_DYNAMIC_DRAW);

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

    m_positions.reserve(config->swarm_size);
    m_position_updates.reserve(config->swarm_size);
    m_orientations.reserve(config->swarm_size);
    m_scales.reserve(config->swarm_size);
    m_neighbors.reserve(config->swarm_size * 4);
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
        m_positions[i] = config->swarm_start + pos;
        m_orientations[i] = glm::vec3{1, 1, 1};
        m_scales.push_back({1, 1, 1});
    }

    // all birds get 4 assigned neighbors
    auto gen_neighbor = std::uniform_int_distribution<size_t>(0, count - 1);
    for (int i = 0; i < count; i++) {
        int tmp = 0;
        for (int j = 0; j < count && tmp != 4; j++) {
            if (i != j) {
                m_neighbors[i * 4 + tmp] = j;
                tmp++;
            }
        }
    }

    calculate_swarm_center();
}

void Swarm::calculate_swarm_center() {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    for (int i = 0; i < config->swarm_size; i++) {
        x += (double)m_positions[i].x / config->swarm_size;
        y += (double)m_positions[i].y / config->swarm_size;
        z += (double)m_positions[i].z / config->swarm_size;
    }

    m_swarm_center = {x, y, z};
}

void Swarm::render(Camera &camera) {
    if (config->debug("trace_swarm")) {
        std::cout << "Swarm::render" << std::endl;
    }
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
                     m_positions.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, instance_dir_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * config->swarm_size,
                     m_orientations.data(), GL_DYNAMIC_DRAW);

        // draw swarm
        glBindVertexArray(bird_mesh.vao);
        glDrawElementsInstanced(GL_TRIANGLES, bird_mesh.indicesCount,
                                GL_UNSIGNED_INT, 0, config->swarm_size);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else {
        for (int i = 0; i < config->swarm_size; i++) {
            auto model =
                glm::lookAt(m_positions[i], m_positions[i] - m_orientations[i],
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
    glDisable(GL_DEPTH_TEST);
    glUseProgram(center_shader);
    viewLocation = glGetUniformLocation(center_shader, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE,
                       glm::value_ptr(camera.GetTransform()));
    projectionLocation = glGetUniformLocation(center_shader, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE,
                       glm::value_ptr(camera.GetProjection()));
    colorLocation = glGetUniformLocation(center_shader, "color");
    glUniform3fv(colorLocation, 1, glm::value_ptr(center_color));

    auto center = Transform(m_swarm_center, glm::vec3{0, 0, 0},
                            glm::vec3{config->debug_sphere_size,
                                      config->debug_sphere_size,
                                      config->debug_sphere_size});
    int modelLocation = glGetUniformLocation(center_shader, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE,
                       glm::value_ptr(center.GetMatrix()));
    glBindVertexArray(center_mesh.vao);
    glDrawElements(GL_TRIANGLES, center_mesh.indicesCount, GL_UNSIGNED_INT, 0);
    glEnable(GL_DEPTH_TEST);
}

void Swarm::smallest_dist() {
    if (config->debug("trace_swarm")) {
        std::cout << "Swarm::smallest_dist" << std::endl;
    }
    auto dist_min = glm::length(m_positions[0] - m_positions[1]);
    auto dist_max = glm::length(m_positions[0] - m_positions[1]);
    auto collisions = 0;
    for (int i = 0; i < config->swarm_size; i++) {
        for (int j = 0; j < config->swarm_size; j++) {
            if (i == j) {
                continue;
            }
            auto dist = glm::length(m_positions[i] - m_positions[j]);
            if (dist < 5.0) {
                collisions++;
            }
            dist_min = std::min(dist_min, dist);
            dist_max = std::max(dist_max, dist);
        }
    }
    std::cout << "Distance min:" << dist_min << " max:" << dist_max
              << " collisions: " << collisions / 2 << std::endl;
}

/////////////////////////////////////////////////////////////////////////////
// CPU Implementation
/////////////////////////////////////////////////////////////////////////////

void Swarm::simulate_tick_cpu(int tick, glm::vec3 track_point, Wind wind,
                              Gravitation gravitation, Enemy enemy) {
    if (config->debug("trace_swarm")) {
        std::cout << "Swarm::simulate_tick_cpu" << std::endl;
    }
    if (config->debug("use_incremental_neighbor_update") && tick != 0) {
        update_neighbors_incremental_cpu();
    } else {
        if (config->debug("use_incremental_neighbor_update") &&
            config->debug("skip_full_neighbor_update")) {
            update_neighbors_incremental_cpu();
        } else {
            update_neighbors_cpu();
        }
    }
    simulate_cpu_external_forces(wind, gravitation);
    simulate_cpu_members(track_point, enemy);
}

void Swarm::update_neighbors_cpu() {
    if (config->debug("use_incremental_neighbor_update") &&
        config->debug("skip_full_neighbor_update")) {
        return;
    }
    if (config->debug("trace_swarm")) {
        std::cout << "Swarm::update_neighbors_cpu" << std::endl;
    }
    config->update_neighbors_cpu.Start();
    for (int i = 0; i < config->swarm_size; i++) {
        auto pos = m_positions[i];
        std::vector<size_t> neigbours = {};
        std::vector<float> distances = {};

        for (auto n = 0; n < 4; n++) {
            neigbours.push_back(m_neighbors[i * 4 + n]);
            distances.push_back(
                glm::length(m_positions[m_neighbors[i * 4 + n]] - pos));
        }

        int largest = 0;
        for (int j = 1; j < 4; j++) {
            if (distances[largest] < distances[j]) {
                largest = j;
            }
        }

        for (int j = 4; j < config->swarm_size; j++) {
            auto dist = glm::length(m_positions[j] - pos);
            if (i != j && j != neigbours[0] && j != neigbours[1] &&
                j != neigbours[2] && j != neigbours[3] &&
                dist < distances[largest]) {
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
        for (auto n = 0; n < 4; n++) {
            m_neighbors[i * 4 + n] = neigbours[n];
        }
    }
    config->update_neighbors_cpu.Stop();
}

void Swarm::update_neighbors_incremental_cpu() {
    if (config->debug("trace_swarm")) {
        std::cout << "Swarm::update_neighbors_incremental_cpu" << std::endl;
    }
    config->update_neighbors_incremental_cpu.Start();
    for (int i = 0; i < config->swarm_size; i++) {
        auto pos = m_positions[i];
        std::vector<size_t> neigbours = {};
        std::vector<float> distances = {};

        for (auto n = 0; n < 4; n++) {
            neigbours.push_back(m_neighbors[i * 4 + n]);
            distances.push_back(
                glm::length(m_positions[m_neighbors[i * 4 + n]] - pos));
        }

        int largest = 0;
        for (int j = 1; j < 4; j++) {
            if (distances[largest] < distances[j]) {
                largest = j;
            }
        }

        // check neighbors of neighbors
        for (auto n = 0; n < 4; n++) {
            for (auto nn = 0; nn < 4; nn++) {
                auto nn_index = n * 4 + nn;
                auto dist = glm::length(m_positions[nn_index] - pos);
                if (nn_index != i && nn_index != neigbours[0] &&
                    nn_index != neigbours[1] && nn_index != neigbours[2] &&
                    nn_index != neigbours[3] && dist < distances[largest]) {
                    distances[largest] = dist;
                    neigbours[largest] = nn_index;
                    int largest = 0;
                    for (int j = 1; j < 4; j++) {
                        if (distances[largest] < distances[j]) {
                            largest = j;
                        }
                    }
                }
            }
        }
        for (auto n = 0; n < 4; n++) {
            m_neighbors[i * 4 + n] = neigbours[n];
        }
    }
    config->update_neighbors_incremental_cpu.Stop();
}

void Swarm::simulate_cpu_members(glm::vec3 track_point, Enemy enemy) {
    if (config->debug("trace_swarm")) {
        std::cout << "Swarm::simulate_cpu_members" << std::endl;
    }
    config->update_swarm_cpu.Start();
    // TODO: clean implementation
    std::vector<glm::vec3> position_updates;
    position_updates.reserve(config->swarm_size);

    for (int i = 0; i < config->swarm_size; i++) {
        auto pos = m_positions[i];

        // initalize values used to calculate target direction
        glm::vec3 neighbor_dir = glm::vec3(0);
        float neighbor_factor = 1;

        glm::vec3 enemy_dodge_dir = glm::vec3(0);
        float enemy_dodge_factor = 0;

        glm::vec3 swarm_center_dir = glm::vec3(0);
        float swarm_center_factor = 0;

        glm::vec3 swarm_target_dir = glm::vec3(0);
        float swarm_target_factor = 0;

        auto vec_to_neighbor0 = m_positions[m_neighbors[i * 4 + 0]] - pos;
        auto vec_to_neighbor1 = m_positions[m_neighbors[i * 4 + 1]] - pos;
        auto vec_to_neighbor2 = m_positions[m_neighbors[i * 4 + 2]] - pos;
        auto vec_to_neighbor3 = m_positions[m_neighbors[i * 4 + 3]] - pos;

        auto neighbor_dist_min = 50.0;
        auto neighbor_dist_max = 250.0;

        auto dist = glm::length(vec_to_neighbor0);
        if (dist < neighbor_dist_min) {
            // away from neighbor0 by some amount
            float fact = neighbor_dist_min / dist;
            if (i == 0 && config->debug("print_bird_debug"))
                std::cout << "dist0: " << dist << std::endl;
            neighbor_dir -= glm::normalize(vec_to_neighbor0) * fact;
            neighbor_factor *= 1.777;
        } else if (neighbor_dist_max < dist) {
            // towards neighbor0 by some amount
            float fact = dist / neighbor_dist_max;
            if (i == 0 && config->debug("print_bird_debug"))
                std::cout << "dist0: " << dist << std::endl;
            neighbor_dir += glm::normalize(vec_to_neighbor0) * fact;
            neighbor_factor *= 1.777;
        }

        dist = glm::length(vec_to_neighbor1);
        if (dist < neighbor_dist_min) {
            float fact = neighbor_dist_min / dist;
            if (i == 0 && config->debug("print_bird_debug"))
                std::cout << "dist1: " << dist << std::endl;
            neighbor_dir -= glm::normalize(vec_to_neighbor1) * fact;
            neighbor_factor *= 1.777;
        } else if (neighbor_dist_max < dist) {
            float fact = dist / neighbor_dist_max;
            if (i == 0 && config->debug("print_bird_debug"))
                std::cout << "dist1: " << dist << std::endl;
            neighbor_dir += glm::normalize(vec_to_neighbor1) * fact;
            neighbor_factor *= 1.777;
        }

        dist = glm::length(vec_to_neighbor2);
        if (dist < neighbor_dist_min) {
            float fact = neighbor_dist_min / dist;
            if (i == 0 && config->debug("print_bird_debug"))
                std::cout << "dist2: " << dist << std::endl;
            neighbor_dir -= glm::normalize(vec_to_neighbor2) * fact;
            neighbor_factor *= 1.777;
        } else if (neighbor_dist_max < dist) {
            float fact = dist / neighbor_dist_max;
            if (i == 0 && config->debug("print_bird_debug"))
                std::cout << "dist2: " << dist << std::endl;
            neighbor_dir += glm::normalize(vec_to_neighbor2) * fact;
            neighbor_factor *= 1.777;
        }

        dist = glm::length(vec_to_neighbor3);
        if (dist < neighbor_dist_min) {
            float fact = neighbor_dist_min / dist;
            if (i == 0 && config->debug("print_bird_debug"))
                std::cout << "dist3: " << dist << std::endl;
            neighbor_dir -= glm::normalize(vec_to_neighbor3) * fact;
            neighbor_factor *= 1.777;
        } else if (neighbor_dist_max < dist) {
            float fact = dist / neighbor_dist_max;
            if (i == 0 && config->debug("print_bird_debug"))
                std::cout << "dist3: " << dist << std::endl;
            neighbor_dir += glm::normalize(vec_to_neighbor3) * fact;
            neighbor_factor *= 1.777;
        }

        auto vec_to_enemy = enemy.get_pos() - pos;
        auto dist_to_enemy = glm::length(vec_to_enemy);
        float min_dist_to_enemy =
            300 + 5 + enemy.get_size(); // 5 is size of bird itself

        if (dist_to_enemy < min_dist_to_enemy) {
            enemy_dodge_factor = 30; // bird panics if enemy is to close
            enemy_dodge_dir -= glm::normalize(vec_to_enemy);
        }

        float desired_swarm_radius =
            (int)ceil(pow(config->swarm_size, 1.f / 3.f)) *
            config->swarm_initial_spread;
        auto vec_to_swarm_center = m_swarm_center - pos;
        swarm_center_factor =
            (glm::length(vec_to_swarm_center) / desired_swarm_radius) * 5;
        swarm_center_dir += glm::normalize(vec_to_swarm_center);

        auto vec_to_track_point = track_point - pos;
        swarm_target_dir += glm::normalize(vec_to_track_point);
        if (desired_swarm_radius / 4 < glm::length(vec_to_track_point)) {
            swarm_target_factor = 5;
        } else {
            swarm_target_factor = 1.5;
        }

        // member target dir
        glm::vec3 target_dir =
            neighbor_dir * neighbor_factor * config->swarm_weight_neighbors +
            enemy_dodge_dir * enemy_dodge_factor * config->swarm_weight_enemy +
            swarm_center_dir * swarm_center_factor *
                config->swarm_weight_swarm_center +
            swarm_target_dir * swarm_target_factor *
                config->swarm_weight_track_point;

        if (i == 0 && config->debug("print_bird_debug")) {
            std::cout << "neighbor_factor: "
                      << neighbor_factor * config->swarm_weight_neighbors
                      << "\n"
                      << "enemy_dodge_factor: "
                      << enemy_dodge_factor * config->swarm_weight_enemy << "\n"
                      << "swarm_center_factor: "
                      << swarm_center_factor * config->swarm_weight_swarm_center
                      << "\n"
                      << "swarm_target_factor: "
                      << swarm_target_factor * config->swarm_weight_track_point
                      << std::endl;
        }

        // update current direction towards new target direction
        auto final_direction = m_orientations[i] + glm::normalize(target_dir) *
                                                       0.33f * config->tick;
        final_direction = glm::normalize(final_direction);
        auto pos_update = final_direction * config->swarm_speed * config->tick;

        position_updates[i] = pos_update;
        m_orientations[i] = {final_direction};
    }

    // apply position update
    glm::vec3 update_swarm_center = {0, 0, 0};
    for (int i = 0; i < config->swarm_size; i++) {
        m_positions[i] += position_updates[i];
        update_swarm_center += position_updates[i] / (float)config->swarm_size;
    }
    m_swarm_center += update_swarm_center;

    config->update_swarm_cpu.Stop();
}

void Swarm::simulate_cpu_external_forces(Wind wind, Gravitation gravitation) {
    if (config->debug("trace_swarm")) {
        std::cout << "Swarm::simulate_cpu_external_forces" << std::endl;
    }
    config->update_external_forces_cpu.Start();
    for (int i = 0; i < config->swarm_size; i++) {
        auto pos = m_positions[i];
        pos += wind.get_force() * config->tick;
        pos += gravitation.get_force() * config->tick;
        m_positions[i] = pos;
    }
    m_swarm_center += wind.get_force() * config->tick;
    m_swarm_center += gravitation.get_force() * config->tick;
    config->update_external_forces_cpu.Stop();
}

/////////////////////////////////////////////////////////////////////////////
// GPU Implementation
/////////////////////////////////////////////////////////////////////////////

void Swarm::simulate_tick_gpu(int tick, glm::vec3 track_point, Wind wind,
                              Gravitation gravitation, Enemy enemy,
                              cl::CommandQueue &queue) {
    if (config->debug("trace_swarm")) {
        std::cout << "Swarm::simulate_tick_gpu" << std::endl;
    }
    if (config->debug("use_incremental_neighbor_update") && tick != 0) {
        update_neighbors_incremental_gpu(queue);
    } else {
        if (config->debug("use_incremental_neighbor_update") &&
            config->debug("skip_full_neighbor_update")) {
            update_neighbors_incremental_gpu(queue);
        } else {
            update_neighbors_gpu(queue);
        }
    }
    simulate_gpu_external_forces(wind, gravitation, queue);
    simulate_gpu_members(track_point, enemy, queue);

    config->copy_buffer_gpu.Start();
    queue.enqueueReadBuffer(m_buf_positions, CL_TRUE, 0,
                            sizeof(float) * 3 * config->swarm_size,
                            m_positions.data());

    queue.enqueueReadBuffer(m_buf_directions, CL_TRUE, 0,
                            sizeof(float) * 3 * config->swarm_size,
                            m_orientations.data());
    queue.finish();
    config->copy_buffer_gpu.Stop();
}

void Swarm::update_neighbors_gpu(cl::CommandQueue &queue) {
    if (config->debug("use_incremental_neighbor_update") &&
        config->debug("skip_full_neighbor_update")) {
        return;
    }
    if (config->debug("trace_swarm")) {
        std::cout << "Swarm::update_neighbors_gpu" << std::endl;
    }
    // load data to buffer
    config->update_neighbors_gpu.Start();

    // run kernel
    auto kernel_neighbor = cl::Kernel(m_kernel_neighbor, "update_neighbor");
    kernel_neighbor.setArg(0, m_buf_positions);
    kernel_neighbor.setArg(1, m_buf_neighbors);
    kernel_neighbor.setArg(2, config->swarm_size);
    queue.enqueueNDRangeKernel(kernel_neighbor, cl::NullRange,
                               cl::NDRange(config->swarm_size), cl::NullRange);

    queue.finish();
    config->update_neighbors_gpu.Stop();
}

void Swarm::update_neighbors_incremental_gpu(cl::CommandQueue &queue) {
    if (config->debug("trace_swarm")) {
        std::cout << "Swarm::update_neighbors_incremental_gpu" << std::endl;
    }
    config->update_neighbors_incremental_gpu.Start();

    // run kernel
    auto kernel_neighbor =
        cl::Kernel(m_kernel_neighbor, "update_neighbor_incremental");
    kernel_neighbor.setArg(0, m_buf_positions);
    kernel_neighbor.setArg(1, m_buf_neighbors);
    queue.enqueueNDRangeKernel(kernel_neighbor, cl::NullRange,
                               cl::NDRange(config->swarm_size), cl::NullRange);

    queue.finish();
    config->update_neighbors_incremental_gpu.Stop();
}

void Swarm::simulate_gpu_members(glm::vec3 track_point, Enemy enemy,
                                 cl::CommandQueue &queue) {
    if (config->debug("trace_swarm")) {
        std::cout << "Swarm::simulate_gpu_members" << std::endl;
    }
    config->update_swarm_gpu.Start();
    float en[4] = {enemy.get_pos().x, enemy.get_pos().y, enemy.get_pos().z, 0};
    float sc[4] = {m_swarm_center.x, m_swarm_center.y, m_swarm_center.z, 0};
    float st[4] = {track_point.x, track_point.y, track_point.z, 0};

    float safe_zone_radius = (int)ceil(pow(config->swarm_size, 1.f / 3.f)) *
                             config->swarm_initial_spread;

    auto kernel_swarm_update =
        cl::Kernel(m_kernel_swarm_update, "swarm_update_members");
    kernel_swarm_update.setArg(0, m_buf_positions);
    kernel_swarm_update.setArg(1, m_buf_position_updates);
    kernel_swarm_update.setArg(2, m_buf_directions);
    kernel_swarm_update.setArg(3, m_buf_neighbors);
    kernel_swarm_update.setArg(4, en);
    kernel_swarm_update.setArg(5, enemy.get_size());
    kernel_swarm_update.setArg(6, config->tick);
    kernel_swarm_update.setArg(7, config->swarm_size);
    kernel_swarm_update.setArg(8, config->swarm_initial_spread);
    kernel_swarm_update.setArg(9, sc);
    kernel_swarm_update.setArg(10, st);
    kernel_swarm_update.setArg(11, config->swarm_speed);
    kernel_swarm_update.setArg(12, safe_zone_radius);
    queue.enqueueNDRangeKernel(kernel_swarm_update, cl::NullRange,
                               cl::NDRange(config->swarm_size), cl::NullRange);

    queue.enqueueReadBuffer(m_buf_position_updates, CL_FALSE, 0,
                            sizeof(float) * 3 * config->swarm_size,
                            m_position_updates.data());
    queue.finish();

    glm::vec3 update_swarm_center = {0, 0, 0};
    for (int i = 0; i < config->swarm_size; i++) {
        update_swarm_center +=
            m_position_updates[i] / (float)config->swarm_size;
    }
    m_swarm_center += update_swarm_center;
    config->update_swarm_gpu.Stop();
}

void Swarm::simulate_gpu_external_forces(Wind wind, Gravitation gravitation,
                                         cl::CommandQueue &queue) {
    if (config->debug("trace_swarm")) {
        std::cout << "Swarm::simulate_gpu_external_forces" << std::endl;
    }
    config->update_external_forces_gpu.Start();

    float grav[4] = {gravitation.get_force().x, gravitation.get_force().y,
                     gravitation.get_force().z, 0};
    float win[4] = {wind.get_force().x, wind.get_force().y, wind.get_force().z,
                    0};
    auto kernel_extern_forces_combined =
        cl::Kernel(m_kernel_external_forces, "combined");
    kernel_extern_forces_combined.setArg(0, m_buf_positions);
    kernel_extern_forces_combined.setArg(1, grav);
    kernel_extern_forces_combined.setArg(2, win);
    kernel_extern_forces_combined.setArg(3, config->tick);
    queue.enqueueNDRangeKernel(kernel_extern_forces_combined, cl::NullRange,
                               cl::NDRange(config->swarm_size), cl::NullRange);

    m_swarm_center += wind.get_force() * config->tick;
    m_swarm_center += gravitation.get_force() * config->tick;

    // we need to wait for the queue to finish before exiting this function, as
    // otherwise the grav and win vector would get be deallocated while the
    // kernel runs and opencl doesn't seam to like it in final cleanup
    queue.finish();
    config->update_external_forces_gpu.Stop();
}

void Swarm::create_kernels_and_buffers(cl::Device &device,
                                       cl::Context &context) {
    if (config->debug("trace_swarm")) {
        std::cout << "Swarm::create_kernels_and_buffers" << std::endl;
    }
    int ret;
    m_kernel_neighbor =
        util::getProgram("res/kernel/neighbor.ocl", context, device);
    m_kernel_external_forces =
        util::getProgram("res/kernel/external_forces.ocl", context, device);
    m_kernel_swarm_update =
        util::getProgram("res/kernel/swarm_update.ocl", context, device);
    m_buf_positions = cl::Buffer(
        context, CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_READ_ONLY,
        sizeof(float) * 3 * config->swarm_size, m_positions.data(), &ret);
    if (ret != CL_SUCCESS) {
        std::cout << "CL_ERROR: (m_buf_positions)" << ret << std::endl;
    }
    m_buf_position_updates =
        cl::Buffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_READ_ONLY,
                   sizeof(float) * 3 * config->swarm_size,
                   m_position_updates.data(), &ret);
    if (ret != CL_SUCCESS) {
        std::cout << "CL_ERROR: (m_buf_positions)" << ret << std::endl;
    }
    m_buf_directions = cl::Buffer(
        context, CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_READ_ONLY,
        sizeof(float) * 3 * config->swarm_size, m_orientations.data(), &ret);
    if (ret != CL_SUCCESS) {
        std::cout << "CL_ERROR: (m_buf_positions)" << ret << std::endl;
    }
    m_buf_neighbors = cl::Buffer(
        context, CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_READ_ONLY,
        sizeof(int) * 4 * config->swarm_size, m_neighbors.data(), &ret);
    if (ret != CL_SUCCESS) {
        std::cout << "CL_ERROR: (m_buf_neighbors)"
                  << sizeof(int) * 4 * m_neighbors.size() << "  " << ret
                  << std::endl;
    }
}

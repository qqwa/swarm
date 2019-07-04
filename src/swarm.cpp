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

    m_posistions.reserve(config->swarm_size);
    m_position_updates.reserve(config->swarm_size);
    m_orientations.reserve(config->swarm_size);
    m_scales.reserve(config->swarm_size);
    m_neighbors.reserve(config->swarm_size*4);
}

void Swarm::reset(cl::CommandQueue &queue) {
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
        m_posistions[i] = config->swarm_start + pos;
        m_position_updates[i] = glm::vec3(0, 0, 0);
        m_orientations[i] = glm::vec3{1, 1, 1};
        m_scales.push_back({1, 1, 1});
    }

    // all birds get 4 assigned neighbors
    auto gen_neighbor = std::uniform_int_distribution<size_t>(0, count - 1);
    for (int i = 0; i < count; i++) {
        int tmp = 0;
        for (int j = 0; j < count && tmp != 4; j++) {
            if (i != j) {
                m_neighbors[i*4+tmp] = j;
                tmp++;
            }
        }
    }

    update_swarm_center();
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

void Swarm::simulate_tick_cpu(int tick, glm::vec3 track_point, Wind wind,
                          Gravitation gravitation) {
    if (config->debug("use_incremental_neighbor_update") && tick == 0) {
        update_neighbors_incremental_cpu();
    } else {
        update_neighbors_cpu();
    }
    simulate_cpu(track_point, wind, gravitation);
}

void Swarm::simulate_tick_gpu(int tick, glm::vec3 track_point, Wind wind,
                          Gravitation gravitation, cl::CommandQueue &queue) {
    if (config->debug("use_incremental_neighbor_update") && tick != 0) {
        update_neighbors_incremental_gpu(queue);
    } else {
        update_neighbors_gpu(queue);
    }
    simulate_gpu(track_point, wind, gravitation);

}

void Swarm::update_neighbors_cpu() {
    config->update_neighbors_cpu.Start();
    for (int i = 0; i < config->swarm_size; i++) {
        auto pos = m_posistions[i];
        std::vector<size_t> neigbours = {};
        std::vector<float> distances = {};

        for (auto n = 0; n < 4; n++) {
            neigbours.push_back(m_neighbors[i*4+n]);
            distances.push_back(glm::length(m_posistions[m_neighbors[i*4+n]] - pos));
        }

        int largest = 0;
        for (int j = 1; j < 4; j++) {
            if (distances[largest] < distances[j]) {
                largest = j;
            }
        }

        for (int j = 4; j < config->swarm_size; j++) {
            auto dist = glm::length(m_posistions[j] - pos);
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
            m_neighbors[i*4+n] = neigbours[n];
        }
    }
    config->update_neighbors_cpu.Stop();
}


void Swarm::update_neighbors_gpu(cl::CommandQueue &queue) {
    // load data to buffer
    return;
    config->update_neighbors_gpu.Start();

    // run kernel
    auto kernel_neighbor = cl::Kernel(m_kernel_neighbor, "update_neighbor");
    kernel_neighbor.setArg(0, m_buf_positions);
    kernel_neighbor.setArg(1, m_buf_neighbors);
    kernel_neighbor.setArg(2, config->swarm_size);
    // auto kernel_neighbor = cl::Kernel(m_kernel_neighbor, "test");
    queue.enqueueNDRangeKernel(kernel_neighbor, cl::NullRange, cl::NDRange(config->swarm_size), cl::NullRange);
    queue.finish();

    // load data from buffer
    auto ret = queue.enqueueReadBuffer(m_buf_neighbors,CL_TRUE,0, sizeof(int)*4*config->swarm_size, m_neighbors.data());
    // queue.finish();

    // std::cout << "<CL_MEM_SIZE>: " << m_buf_neighbors.getInfo<CL_MEM_SIZE>() << std::endl;
    // void* mapped_read = queue.enqueueMapBuffer(m_buf_neighbors, CL_TRUE, CL_MAP_READ, 0, 10);
    // std::cout << "Mapped value of Buffer C at index 1: " << ((cl_int *)mapped_read)[1] << std::endl;
    // queue.enqueueUnmapMemObject(m_buf_neighbors, mapped_read);
    // for (int i = 0; i < 4; i++) {
    //     std::cout << "\t" << m_neighbors[i] << "\n";
    // }
    // std::cout << std::endl;
    // exit(1);

    config->update_neighbors_gpu.Stop();
};

// swarm disappears after some time? o.O
void Swarm::update_neighbors_incremental_cpu() {
    config->update_neighbors_incremental_cpu.Start();
    for (int i = 0; i < config->swarm_size; i++) {
        auto pos = m_posistions[i];
        std::vector<size_t> neigbours = {};
        std::vector<float> distances = {};

        for (auto n = 0; n < 4; n++) {
            neigbours.push_back(m_neighbors[i*4+n]);
            distances.push_back(glm::length(m_posistions[m_neighbors[i*4+n]] - pos));
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
                auto nn_index = n*4+nn;
                auto dist = glm::length(m_posistions[nn_index] - pos);
                if (nn_index != i && nn_index != neigbours[0] && nn_index != neigbours[1] &&
                    nn_index != neigbours[2] && nn_index != neigbours[3] &&
                    dist < distances[largest]) {
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
            m_neighbors[i*4+n] = neigbours[n];
        }
    }
    config->update_neighbors_incremental_cpu.Stop();
}

void Swarm::update_neighbors_incremental_gpu(cl::CommandQueue &queue) {
    config->update_neighbors_incremental_gpu.Start();

    // run kernel
    auto kernel_neighbor = cl::Kernel(m_kernel_neighbor, "update_neighbor_incremental");
    kernel_neighbor.setArg(0, m_buf_positions);
    kernel_neighbor.setArg(1, m_buf_neighbors);
    queue.enqueueNDRangeKernel(kernel_neighbor, cl::NullRange, cl::NDRange(config->swarm_size), cl::NullRange);
    queue.finish();

    auto ret = queue.enqueueReadBuffer(m_buf_neighbors,CL_TRUE,0, sizeof(int)*4*config->swarm_size, m_neighbors.data());

    config->update_neighbors_incremental_gpu.Stop();
}

// programmed as it were a "kernel"
void Swarm::simulate_cpu(glm::vec3 track_point, Wind wind,
                         Gravitation gravitation) {
    config->update_swarm_cpu.Start();
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
        // 1. try to keep distance x between all neighbors
        /////////////////////////////////////////////////////////////////////////////
        auto neighbor0 = m_posistions[m_neighbors[i*4+0]] - pos;
        auto neighbor1 = m_posistions[m_neighbors[i*4+1]] - pos;
        auto neighbor2 = m_posistions[m_neighbors[i*4+2]] - pos;
        auto neighbor3 = m_posistions[m_neighbors[i*4+3]] - pos;

        auto dist_neighbor0 = glm::length(neighbor0);
        auto dist_neighbor1 = glm::length(neighbor1);
        auto dist_neighbor2 = glm::length(neighbor2);
        auto dist_neighbor3 = glm::length(neighbor3);

        auto vec_neigbour0 = glm::vec3(0, 0, 0);
        auto vec_neigbour1 = glm::vec3(0, 0, 0);
        auto vec_neigbour2 = glm::vec3(0, 0, 0);
        auto vec_neigbour3 = glm::vec3(0, 0, 0);

        auto neighbor_dist_factor = 5.0;
        auto neighbor_dist_ideal = 250.0;
        auto neighbor_dist_toleration = 100.0;

        if (dist_neighbor0 < neighbor_dist_ideal - neighbor_dist_toleration ||
            neighbor_dist_ideal + neighbor_dist_toleration < dist_neighbor0) {
            vec_neigbour0 =
                neighbor0 * (float)((dist_neighbor0 - neighbor_dist_ideal) *
                                    neighbor_dist_factor);
        }

        if (dist_neighbor1 < neighbor_dist_ideal - neighbor_dist_toleration ||
            neighbor_dist_ideal + neighbor_dist_toleration < dist_neighbor1) {
            vec_neigbour1 =
                neighbor1 * (float)((dist_neighbor1 - neighbor_dist_ideal) *
                                    neighbor_dist_factor);
        }

        if (dist_neighbor2 < neighbor_dist_ideal - neighbor_dist_toleration ||
            neighbor_dist_ideal + neighbor_dist_toleration < dist_neighbor2) {
            vec_neigbour2 =
                neighbor2 * (float)((dist_neighbor2 - neighbor_dist_ideal) *
                                    neighbor_dist_factor);
        }

        if (dist_neighbor3 < neighbor_dist_ideal - neighbor_dist_toleration ||
            neighbor_dist_ideal + neighbor_dist_toleration < dist_neighbor3) {
            vec_neigbour3 =
                neighbor3 * (float)((dist_neighbor3 - neighbor_dist_ideal) *
                                    neighbor_dist_factor);
        }

        auto neihgour_factor =
            std::max(glm::length(vec_neigbour0 + vec_neigbour1 + vec_neigbour2 +
                                 vec_neigbour3) /
                         4.0f,
                     10.0f);

        auto neighbor_correction =
            vec_neigbour0 + vec_neigbour1 + vec_neigbour2 + vec_neigbour3;

        /////////////////////////////////////////////////////////////////////////////
        // 2. try to center between neighbors
        /////////////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////////////
        // 3. fly in direction of track point
        /////////////////////////////////////////////////////////////////////////////
        // dont normalize because the length matters?
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
        if (glm::length(neighbor_correction) != 0) {
            neighbor_correction = glm::normalize(neighbor_correction);
        }
        if (glm::length(tp_direction) != 0) {
            tp_direction = glm::normalize(tp_direction);
        }
        if (glm::length(swarm_center_direction) != 0) {
            swarm_center_direction = glm::normalize(swarm_center_direction);
        }

        neighbor_correction *= config->swarm_weight_neighbors;
        tp_direction *= config->swarm_weight_track_point;
        swarm_center_direction *= config->swarm_weight_swarm_center;

        auto target_direction = glm::normalize(
            // 25.0f * spread_direction * glm::length(m_swarm_center - pos) +
            neighbor_correction + tp_direction + swarm_center_direction);

        // TODO: acceleration, realistic direction change, etc..
        auto final_direction =
            m_orientations[i] + target_direction * 0.33f * config->tick;
        final_direction = glm::normalize(final_direction);
        auto pos_update = final_direction * config->swarm_speed * config->tick;

        m_position_updates[i] = pos_update;
        m_orientations[i] = {final_direction};

        if (i == 0 && config->debug("print_bird_vectors")) {
            std::cout << "pos: " << glm::to_string(pos) << std::endl
                      << "center: " << glm::to_string(m_swarm_center)
                      << std::endl
                      << "neighbor_correction: "
                      << glm::to_string(neighbor_correction) << std::endl
                      << "  n0 (" << glm::to_string(neighbor0)
                      << "):" << dist_neighbor0 << std::endl
                      << "  n1 (" << glm::to_string(neighbor1)
                      << "):" << dist_neighbor1 << std::endl
                      << "  n2 (" << glm::to_string(neighbor2)
                      << "):" << dist_neighbor2 << std::endl
                      << "  n3 (" << glm::to_string(neighbor3)
                      << "):" << dist_neighbor3 << std::endl
                      << "center_direction:     "
                      << glm::to_string(swarm_center_direction) << std::endl
                      << "tp_direction:         "
                      << glm::to_string(tp_direction) << std::endl
                      << "pos_update            " << glm::to_string(pos_update)
                      << std::endl
                      << "final_direction       "
                      << glm::to_string(final_direction) << std::endl
                      << "" << std::endl;
        }
    }
    /////////////////////////////////////////////////////////////////////////////
    // 7. update swarm center, with change vector of all members
    /////////////////////////////////////////////////////////////////////////////
    for (int i = 0; i < config->swarm_size; i++) {
        m_posistions[i] += m_position_updates[i];
        update_swarm_center +=
            m_position_updates[i] / (float)config->swarm_size;
    }
    m_swarm_center += update_swarm_center;

    config->update_swarm_cpu.Stop();
}

void Swarm::simulate_gpu(glm::vec3 track_point, Wind wind,
                         Gravitation gravitation) {
    // TODO: use gpu
    simulate_cpu(track_point, wind, gravitation);

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
        for (int i = 0; i < config->swarm_size; i++) {
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

    auto center =
        Transform(m_swarm_center, glm::vec3{0, 0, 0}, glm::vec3{config->sphere_size,config->sphere_size,config->sphere_size});
    int modelLocation = glGetUniformLocation(center_shader, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE,
                       glm::value_ptr(center.GetMatrix()));
    glBindVertexArray(center_mesh.vao);
    glDrawElements(GL_TRIANGLES, center_mesh.indicesCount, GL_UNSIGNED_INT, 0);
    glEnable(GL_DEPTH_TEST);
}


void Swarm::create_kernels_and_buffers(cl::Device &device, cl::Context &context) {
    int ret;
    m_kernel_neighbor = util::getProgram("res/kernel/neighbor.ocl", context, device);
    m_buf_positions = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(float)*3*config->swarm_size, m_posistions.data(), &ret);
    if (ret != CL_SUCCESS) {
        std::cout << "CL_ERROR: (m_buf_positions)" << ret << std::endl;
    }
    m_buf_directions;
    m_buf_neighbors = cl::Buffer(context, CL_MEM_USE_HOST_PTR | CL_MEM_HOST_READ_ONLY, sizeof(int)*4*config->swarm_size, m_neighbors.data(), &ret);
    if (ret != CL_SUCCESS) {
        std::cout << "CL_ERROR: (m_buf_neighbors)" << sizeof(int)*4*m_neighbors.size() << "  " << ret << std::endl;
    }
}


void Swarm::smallest_dist() {
    auto dist_min = glm::length(m_posistions[0] - m_posistions[1]); 
    auto dist_max = glm::length(m_posistions[0] - m_posistions[1]);
    auto collisions = 0;
    for (int i = 0; i < config->swarm_size; i++) {
        for(int j = 0; j < config->swarm_size; j++) {
            if (i==j) {
                continue;
            }
            auto dist = glm::length(m_posistions[i] - m_posistions[j]);
            if (dist < 5.0) {
                collisions++;
            }
            dist_min = std::min(dist_min, dist);
            dist_max = std::max(dist_max, dist);
        }
    }
    std::cout << "Distance min:" << dist_min << " max:" << dist_max << " collisions: " << collisions << std::endl;
}
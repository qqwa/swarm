#pragma once

#include "camera.h"
#include "gravitation.h"
#include "transform.h"
#include "util.h"
#include "wind.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <random>
#include <vector>
#include <CL/cl.hpp>

class Swarm {
  private:
    glm::vec3 bird_color;
    util::MeshMetaData bird_mesh;
    GLuint bird_shader;
    GLuint instance_pos_vbo;
    GLuint instance_dir_vbo;

    cl::Program m_kernel_neighbor;
    cl::Buffer m_buf_positions;
    cl::Buffer m_buf_directions;
    cl::Buffer m_buf_neighbors;

    glm::vec3 center_color;
    util::MeshMetaData center_mesh;
    GLuint center_shader;

    std::vector<glm::vec3> m_posistions;
    std::vector<glm::vec3> m_position_updates;
    std::vector<glm::vec3> m_orientations;
    std::vector<glm::vec3> m_scales; // most likly the same for all members..
    std::vector<int> m_neighbors;
    glm::vec3 m_swarm_center;

    std::mt19937_64 m_random;
    void update_swarm_center();
    void update_neighbors_cpu();
    void update_neighbors_gpu(cl::CommandQueue &queue);
    void update_neighbors_incremental_cpu();
    void update_neighbors_incremental_gpu(cl::CommandQueue &queue);
    void simulate_cpu(glm::vec3 track_point, Wind wind,
                      Gravitation gravitation);
    void simulate_gpu(glm::vec3 track_point, Wind wind,
                      Gravitation gravitation);

  public:
    Swarm();
    void create_kernels_and_buffers(cl::Device &device, cl::Context &context);
    void reset(cl::CommandQueue &queue);
    size_t size();
    void simulate_tick_cpu(int tick, glm::vec3 track_point, Wind wind,
                       Gravitation gravitation);
    void simulate_tick_gpu(int tick, glm::vec3 track_point, Wind wind,
                       Gravitation gravitation, cl::CommandQueue &queue);
    void render(Camera &camera);
    void smallest_dist();
};

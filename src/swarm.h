#pragma once

#include "camera.h"
#include "enemy.h"
#include "gravitation.h"
#include "transform.h"
#include "util.h"
#include "wind.h"
#include <CL/cl.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <random>
#include <vector>

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
    std::vector<glm::vec3> m_orientations;
    std::vector<glm::vec3> m_scales; // most likly the same for all members..
    std::vector<int> m_neighbors;
    glm::vec3 m_swarm_center;

    std::mt19937_64 m_random;
    void calculate_swarm_center();

    void simulate_cpu(glm::vec3 track_point, Wind wind,
                      Gravitation gravitation);
    void simulate_cpu_v2(glm::vec3 track_point, Wind wind,
                         Gravitation gravitation, Enemy enemy);
    void update_neighbors_cpu();
    void update_neighbors_incremental_cpu();

    void simulate_gpu(glm::vec3 track_point, Wind wind, Gravitation gravitation,
                      Enemy enemy, cl::CommandQueue &queue);
    void update_neighbors_gpu(cl::CommandQueue &queue);
    void update_neighbors_incremental_gpu(cl::CommandQueue &queue);

  public:
    Swarm();

    /**
     * @brief Sets initial position, direction etc. of swarm members
     */
    void reset();
    /**
     * @brief Run one tick on CPU
     * @param tick Current tick of the simulation, currently only used if we
     * need to do large calculations only once on the first tick
     * @param track_point Point which the members of the swarm want to reach
     * @param wind Wind that gets applied to all swarm members
     * @param gravitation Gravitation that gets applied to all swarm members
     * @param enemy Enemy which the swarm members want to avoid
     */

    void simulate_tick_cpu(int tick, glm::vec3 track_point, Wind wind,
                           Gravitation gravitation, Enemy enemy);

    /**
     * @brief Run one tick on GPU, before this method is called
     * `create_kernels_and_buffers` needs to be called.
     * @param tick Current tick of the simulation, currently only used if we
     * need to do large calculations only once on the first tick
     * @param track_point Point which the members of the swarm want to reach
     * @param wind Wind that gets applied to all swarm members
     * @param gravitation Gravitation that gets applied to all swarm members
     * @param enemy Enemy which the swarm members want to avoid
     * @param queue OpenCL Command Queue to execute kernels
     */
    void simulate_tick_gpu(int tick, glm::vec3 track_point, Wind wind,
                           Gravitation gravitation, Enemy enemy,
                           cl::CommandQueue &queue);

    /**
     * @brief Creates OpenCL Kernels and Buffers, this function needs to be
     * called only once
     * @param device OpenCL Device
     * @param context OpenCL Context
     */
    void create_kernels_and_buffers(cl::Device &device, cl::Context &context);

    /**
     * @brief Renders all swarm members and also the swarm center
     * @param camera Camera to get the view and projection matrices for the
     * shader
     */
    void render(Camera &camera);

    /**
     * @brief Brute-force checks potential collisions between swarm members, bad
     * for perfomance and should only be used to check how good the collision
     * avoidance between swarm members works.
     */
    void smallest_dist();
};

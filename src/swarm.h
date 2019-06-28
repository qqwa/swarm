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

class Swarm {
  private:
    glm::vec3 bird_color;
    util::MeshMetaData bird_mesh;
    GLuint bird_shader;
    GLuint instance_pos_vbo;
    GLuint instance_dir_vbo;

    glm::vec3 center_color;
    util::MeshMetaData center_mesh;
    GLuint center_shader;

    std::vector<glm::vec3> m_posistions;
    std::vector<glm::vec3> m_orientations;
    std::vector<glm::vec3> m_scales; // most likly the same for all members..
    std::vector<std::vector<size_t>> m_neighbors;
    glm::vec3 m_swarm_center;

    std::mt19937_64 m_random;
    void update_swarm_center();
    void update_neighbours();
    void simulate_cpu(glm::vec3 track_point, Wind wind,
                      Gravitation gravitation);
    void simulate_gpu();

  public:
    Swarm();
    Swarm(size_t member_count);
    void reset();
    size_t size();
    void simulate_tick(glm::vec3 track_point, Wind wind,
                       Gravitation gravitation);
    void render(Camera &camera);
};

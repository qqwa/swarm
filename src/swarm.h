#pragma once

#include "camera.h"
#include "transform.h"
#include "util.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <random>
#include <vector>

class Swarm {
  private:
    glm::vec3 bird_color;
    util::MeshMetaData bird_mesh;
    GLuint bird_shader;


    glm::vec3 center_color;
    util::MeshMetaData center_mesh;
    GLuint center_shader;

    std::vector<glm::vec3> m_posistions;
    std::vector<glm::quat> m_orientations;
    std::vector<glm::vec3> m_scales; // most likly the same for all members..
    std::vector<std::vector<size_t>> m_neighbors;
    glm::vec3 m_swarm_center;

    std::mt19937_64 m_random;
    void update_swarm_center();
    void simulate_cpu(glm::vec3 track_point);
    void simulate_gpu();

  public:
    Swarm();
    Swarm(size_t member_count);
    void reset();
    size_t size();
    void simulate_tick(glm::vec3 track_point);
    void update(float delta);
    void render(Camera &camera);
};

#pragma once

#include "camera.h"
#include "transform.h"
#include "util.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

class Swarm {
  private:
    glm::vec3 bird_color;
    util::MeshMetaData bird_mesh;
    GLuint bird_shader;

    std::vector<glm::vec3> m_posistions;
    std::vector<glm::quat> m_orientations;
    std::vector<glm::vec3> m_scales; // most likly the same for all members..
  public:
    Swarm();
    Swarm(size_t member_count);
    void reset_transforms(glm::vec3 position, glm::quat orientation,
                          glm::vec3 scale);
    size_t size();
    void update(float delta);
    void render(Camera &camera);
};

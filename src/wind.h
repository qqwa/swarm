#pragma once

#include "camera.h"
#include "transform.h"
#include "util.h"

class Wind {
  private:
    glm::vec3 arrow_color;
    util::MeshMetaData arrow_mesh;
    GLuint arrow_shader;

    glm::quat m_direction;
    float m_strength;

  public:
    Wind();
    void update(int tick);
    void render(Camera &camera);
    glm::vec3 get_force() const;
};

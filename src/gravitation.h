#pragma once

#include "camera.h"
#include "transform.h"
#include "util.h"

class Gravitation {
  private:
    glm::vec3 arrow_color;
    util::MeshMetaData arrow_mesh;
    GLuint arrow_shader;

    glm::quat m_direction;
    float m_strength;

  public:
    Gravitation();
    Gravitation(float force);
    void update(float delta);
    void render(Camera &camera);
    glm::vec3 get_force() const;
};

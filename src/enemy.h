#pragma once

#include "camera.h"
#include "transform.h"
#include "util.h"

class Enemy {
  private:
    glm::vec3 enemy_color;
    util::MeshMetaData enemy_mesh;
    GLuint enemy_shader;

    Transform m_transform;

  public:
    Enemy();
    void update(int tick);
    void render(Camera &camera);
    glm::vec3 get_pos() const;
};

#pragma once

#include "camera.h"
#include "debug_defines.h"
#include "transform.h"
#include "util.h"

class Wind {
  private:
    glm::vec3 arrow_color;
    util::MeshMetaData arrow_mesh;
    GLuint arrow_shader;

    Transform m_transform;

  public:
    Wind();
    void update(float delta);
    void render(Camera &camera);
};

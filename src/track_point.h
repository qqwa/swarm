#pragma once

#include "camera.h"
#include "transform.h"
#include "util.h"

class TrackPoint {
  private:
    glm::vec3 track_point_color;
    util::MeshMetaData track_point_mesh;
    GLuint track_point_shader;

    Transform m_transform;

  public:
    TrackPoint();
    void update(int tick);
    void render(Camera &camera);
    glm::vec3 get_pos() const;
};

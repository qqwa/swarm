#pragma once

#include "transform.h"
#include <glm/glm.hpp>

class Camera {
  private:
    glm::mat4 projection;
    Transform transform;

  public:
    Camera(glm::mat4 projection, Transform transform);
    const glm::mat4 &GetProjection() const;
    glm::mat4 GetTransform() const;
};
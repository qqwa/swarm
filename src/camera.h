#pragma once

#include "debug_defines.h"
#include <glm/glm.hpp>

class Camera {
  private:
    glm::mat4 m_projection;
    glm::vec3 m_position;
    float m_yaw;
    float m_pitch;
    glm::vec3 forward() const;

  public:
    Camera();
    Camera(glm::mat4 projection);
    void setProjection(glm::mat4 projection);
    const glm::mat4 &GetProjection() const;
    glm::mat4 GetTransform() const;
    glm::mat4 GetTransformWithoutTranslation() const;
    void move_forward(float value);
    void move_left(float value);
    void rotateHorizontal(float value);
    void rotateVertical(float value);
};
#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

Camera::Camera()
    : m_projection(glm::mat4(1)), m_position(0), m_pitch(0), m_yaw(0) {}

Camera::Camera(glm::mat4 projection) : m_projection(projection) {}

void Camera::setProjection(glm::mat4 projection) { m_projection = projection; }

const glm::mat4 &Camera::GetProjection() const { return m_projection; }

glm::vec3 Camera::forward() const {
    glm::vec3 forward;
    forward.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    forward.y = sin(glm::radians(m_pitch));
    forward.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    return forward;
}

glm::mat4 Camera::GetTransform() const {
#ifdef DEBUG_CAMERA
    std::cout << "Pos: " << glm::to_string(m_position)
              << " - Pitch: " << m_pitch << " - Yaw: " << m_yaw << std::endl;
#endif DEBUG_CAMERA
    return glm::lookAt(m_position, m_position + forward(), {0.0f, 1.0f, 0.0f});
}

void Camera::move_forward(float value) { m_position += forward() * value; }

void Camera::move_left(float value) {
    m_position += glm::normalize(glm::cross(forward(), {0, -1, 0})) * value;
}

void Camera::rotateHorizontal(float value) {
    m_pitch += value;
    if (m_pitch < -89.f) {
        m_pitch = -89.f;
    }
    if (89.f < m_pitch) {
        m_pitch = 89.f;
    }
}

void Camera::rotateVertical(float value) { m_yaw += value; }

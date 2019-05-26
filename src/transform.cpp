#include "transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

Transform::Transform(const glm::vec3 &position, const glm::quat &orientation,
                     const glm::vec3 &scale)
    : m_position(position), m_orientation(orientation), m_scale(scale) {}

Transform::Transform(const glm::vec3 &position)
    : Transform(position, glm::quat(), {1.0f, 1.0f, 1.0f}) {}

Transform::Transform(const Transform &transform) {
    m_position = transform.m_position;
    m_orientation = transform.m_orientation;
    m_scale = transform.m_scale;
}

Transform::~Transform() {}

glm::mat4 Transform::Matrix(const glm::vec3 &position,
                            const glm::quat &orientation,
                            const glm::vec3 &scale) {
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1), position);
    glm::mat4 rotationMatrix = glm::mat4_cast(orientation);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1), scale);
    return translationMatrix * rotationMatrix * scaleMatrix;
}

const glm::mat4 Transform::GetMatrix() const {
    return Matrix(m_position, m_orientation, m_scale);
}

void Transform::Translate(const glm::vec3 &translation) {
    m_position += translation;
}

void Transform::Rotate(const float &degree, const glm::vec3 &axis) {
    m_orientation = glm::rotate(m_orientation, glm::radians(degree), axis);
    m_orientation = glm::normalize(m_orientation);
}

void Transform::Scale(const glm::vec3 &scale) {
    m_scale.x *= scale.x;
    m_scale.y *= scale.y;
    m_scale.z *= scale.z;
}

void Transform::SetPosition(const glm::vec3 &positon) { m_position = positon; }

void Transform::SetOrientation(const glm::quat &orientation) {
    m_orientation = orientation;
}

void Transform::SetScale(const glm::vec3 &scale) { m_scale = scale; }

const glm::vec3 &Transform::GetPosition() const { return m_position; }

glm::vec3 Transform::GetPosition() { return m_position; }

const glm::quat &Transform::GetOrientation() const { return m_orientation; }

glm::quat &Transform::GetOrientation() { return m_orientation; }

const glm::vec3 &Transform::GetScale() const { return m_scale; }

glm::vec3 Transform::GetScale() { return m_scale; }

glm::vec3 Transform::GetForward() const {
    glm::vec3 FORWARD{0.0f, 0.0f, -1.0f};
    return glm::normalize((m_orientation * FORWARD));
}

glm::vec3 Transform::GetUp() const {
    glm::vec3 UPWARD{0.0f, 1.0f, 0.0f};
    return glm::normalize((m_orientation * UPWARD));
}

glm::vec3 Transform::GetRight() const {
    return glm::normalize(m_orientation * (glm::cross(GetForward(), GetUp())));
}
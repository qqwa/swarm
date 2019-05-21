#include "camera.h"


Camera::Camera(glm::mat4 projection, Transform transform)
: projection(projection), transform(transform) {}

const glm::mat4 &Camera::GetProjection() const {
    return projection;
}

glm::mat4 Camera::GetTransform() const {
    return glm::inverse(transform.GetMatrix());
}
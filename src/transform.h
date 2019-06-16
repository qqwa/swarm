#pragma once

#include "debug_defines.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

/**
 * @brief Transform contains all necessary information about an Entities
 * location.
 *
 * It will be used by the RenderingSystem and the CollisionSystem. Furthermore
 * it can be useful for Scripts.
 */
class Transform {
    glm::vec3 m_position;
    glm::quat m_orientation;
    glm::vec3 m_scale;

  public:
    typedef std::shared_ptr<Transform> Ptr;

    explicit Transform(const glm::vec3 &position, const glm::quat &orientation,
                       const glm::vec3 &scale);

    explicit Transform(const glm::vec3 &position);

    Transform(const Transform &);

    ~Transform();

    static glm::mat4 Matrix(const glm::vec3 &posistion,
                            const glm::quat &orientation,
                            const glm::vec3 &scale);

    const glm::mat4 GetMatrix() const;

    void Translate(const glm::vec3 &translation);

    void Rotate(const float &degree, const glm::vec3 &axis);

    void Scale(const glm::vec3 &scale);

    void SetPosition(const glm::vec3 &positon);

    void SetOrientation(const glm::quat &orientation);

    void SetScale(const glm::vec3 &scale);

    const glm::vec3 &GetPosition() const;

    glm::vec3 GetPosition();

    const glm::quat &GetOrientation() const;

    glm::quat &GetOrientation();

    glm::quat GetTotalOrientation();

    const glm::vec3 &GetScale() const;

    glm::vec3 GetScale();

    glm::vec3 GetForward() const;

    glm::vec3 GetUp() const;

    glm::vec3 GetRight() const;
};

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

class Swarm {
    private:
    std::vector<glm::vec3> m_posistions;
    std::vector<glm::quat> m_orientations;
    std::vector<glm::vec3> m_scales;    // most likly the same for all members..
    public:
    Swarm(size_t member_count, glm::vec3 position);
};

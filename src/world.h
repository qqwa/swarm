#pragma once

#include <glm/glm.hpp>
#include "util.h"

class World {
    private:
    glm::vec3 clear_color;
    glm::vec3 bird_color;
    util::MeshMetaData bird;

    public:
    World();
    void update(float delta);
    void render();
};

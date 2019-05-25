#pragma once

#include "util.h"
#include <glm/glm.hpp>

class World {
  private:
    glm::vec3 clear_color;
    glm::vec3 bird_color;
    util::MeshMetaData bird;
    GLuint bird_shader;

    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 model;

  public:
    World();
    void update(float delta);
    void render();
};

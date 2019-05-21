#include "world.h"
#include <GL/gl.h>
#include <iostream>

World::World() {
    clear_color = {0.2, 0.3, 0.3};
    bird_color = {0.0, 0.0, 0.0};
    bird = util::loadMesh("res/bird.obj");
}

void World::update(float delta) {

}

void World::render() {
        glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
}
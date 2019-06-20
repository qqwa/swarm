#include <iostream>
#define GLEW_STATIC
#include "config.h"
#include "world.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " CONFIG" << std::endl;
        return 1;
    }
    auto path = std::string(argv[1]);
    config = new Config(path);

    GLFWwindow *window;

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    std::cout << config->random_seed << std::endl;

    window = glfwCreateWindow(config->width, config->height, "DebugWindow",
                              nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create window" << std::endl;
        exit(1);
    }
    glfwMakeContextCurrent(window);

    // TODO: init opengl
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialze GLEW" << std::endl;
        exit(1);
    }

    // TODO: init opencl

    // TODO: initialize data, swarm members, mesh, etc...
    World world = World(window);

    while (!glfwWindowShouldClose(window) &&
           !glfwGetKey(window, GLFW_KEY_ESCAPE)) {
        glfwPollEvents();

        // TODO: check if we need to calculate next frame
        world.update(0.0167);

        // TODO: draw frame
        world.render();

        glfwSwapBuffers(window);
    }

    return 0;
}
#include<iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "world.h"

int main() {
    GLFWwindow* window;

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(800, 600, "DebugWindow", nullptr, nullptr);
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
    World world = World();

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // TODO: check if we need to calculate next frame
        world.update(0.0167);

        // TODO: draw frame
        world.render();

        glfwSwapBuffers(window);
    }

    std::cout << "Hello World!" << std::endl;
    return 0;
}
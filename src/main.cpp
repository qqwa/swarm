#include <iostream>
#define GLEW_STATIC
#include "config.h"
#include "profiler.h"
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
    if (config->debug("vsync_off")) {
        glfwSwapInterval(0);
    }

    // TODO: init opengl
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialze GLEW" << std::endl;
        exit(1);
    }

    // TODO: init opencl
    auto default_device = util::getFirstDevice();
    auto context = cl::Context(default_device);
    auto queue = cl::CommandQueue(context, default_device);

    // TODO: initialize data, swarm members, mesh, etc...
    World world = World(window, &default_device, &context, &queue);

    while (!glfwWindowShouldClose(window) &&
           !glfwGetKey(window, GLFW_KEY_ESCAPE)) {
        glfwPollEvents();

        // TODO: check if we need to calculate next frame
        config->update.Start();
        world.update(0.0167);
        config->update.Stop();

        // TODO: draw frame
        config->render.Start();
        world.render();
        config->render.Stop();

        glfwSwapBuffers(window);
    }

    std::cout << "Profiler output:\n"
              << "========================" << std::endl;
    std::cout << config->update << std::endl;
    std::cout << "\t" << config->update_neighbors << std::endl;
    std::cout << "\t" << config->update_neighbors_incremental << std::endl;
    std::cout << "\t" << config->update_swarm << std::endl;
    std::cout << config->render << std::endl;

    return 0;
}
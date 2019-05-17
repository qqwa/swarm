cmake_minimum_required(VERSION 3.7)
project(gpu-swarm)

add_executable(swarm
    src/main.cpp
)

set_property(TARGET swarm PROPERTY CXX_STANDARD 11)
target_compile_options(swarm PRIVATE -Wall)

# find_package( OpenGL REQUIRED )

# glfw
add_subdirectory(libs/glfw EXCLUDE_FROM_ALL)
target_link_libraries(swarm PRIVATE glfw)

# glew
find_package(GLEW REQUIRED)
target_link_libraries(swarm PRIVATE GLEW::GLEW)

# glm
add_subdirectory(libs/glm EXCLUDE_FROM_ALL)
target_link_libraries(swarm PRIVATE glm)

# opencl
find_package(OpenCL REQUIRED)
target_link_libraries(swarm PRIVATE OpenCL::OpenCL)
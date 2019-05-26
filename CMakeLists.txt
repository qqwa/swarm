cmake_minimum_required(VERSION 3.7)
project(gpu-swarm)

add_executable(swarm
    src/main.cpp
    src/util.cpp
    src/util.h
    src/profiler.cpp
    src/profiler.h
    src/world.cpp
    src/world.h
    src/wind.cpp
    src/wind.h
    src/swarm.cpp
    src/swarm.h
    src/gravitation.cpp
    src/gravitation.h
    src/track_point.cpp
    src/track_point.h
    src/swarm_trajectory.cpp
    src/swarm_trajectory.h
    src/transform.cpp
    src/transform.h
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

# opengl
find_package(OpenGL REQUIRED)
target_link_libraries(swarm PRIVATE ${OPENGL_LIBRARIES})

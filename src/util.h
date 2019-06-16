#pragma once

#include "debug_defines.h"
#include <CL/cl.hpp>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

namespace util {
/**
 * @brief MeshMetaData contains all necessary informations to draw a mesh
 */
struct MeshMetaData {
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    unsigned long indicesCount;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

/**
 * @brief loadFile provides the content of a File as a string
 * @param path The relative or total path to the file
 * @return The content of the file
 */
std::string loadFile(std::string path);

/**
 * @brief getProgram loads and compiles a kernel
 * @param path Path to the kernel code
 * @param context Context in which the kernel will be used
 * @param device Device for which the kernel gets compiled
 * @return cl::Program to execute the kernel
 */
cl::Program getProgram(std::string path, cl::Context &context,
                       cl::Device &device);

/**
 * @brief getFirstDevice returns the first device that can be used with OpenCL,
 * will exit if none ist found
 * @return Device to use OpenCL with
 */
cl::Device getFirstDevice();

/**
 * @brief getShader loads the shader code and crates an OpenGL program
 * @param path Path to the shader code, without file extensions, function will
 * automatically look for `path + .vert` and `path + .frag`
 * @return GLuint of the shader program
 */
GLuint getShader(std::string path);

/**
 * @brief Loads a .obj file and uploads the mesh data to the gpu
 * @param path Path to the mesh
 * @return MeshMetaData to draw the mesh
 */
MeshMetaData loadMesh(std::string path);

/**
 * @brief frees the used memory of a mesh on the GPU
 * @param mesh Mesh to be freed from the GPU
 */
void freeMesh(MeshMetaData mesh);
} // namespace util
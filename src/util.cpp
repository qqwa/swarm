#include "util.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

std::string util::loadFile(std::string path) {
    std::ifstream inFile;
    inFile.open(path); // open the input file

    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::string file_content = strStream.str();

    return file_content;
}

cl::Program util::getProgram(std::string path, cl::Context &context,
                             cl::Device &device) {
    std::string kernel_code = loadFile(path);

    cl::Program::Sources sources;
    sources.push_back({kernel_code.c_str(), kernel_code.length()});

    cl::Program program(context, sources);
    if (program.build({device}) != CL_SUCCESS) {
        std::cout << " Error building: "
                  << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)
                  << std::endl;
        exit(1);
    }

    return program;
}

cl::Device util::getFirstDevice() {
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    if (all_platforms.size() == 0) {
        std::cout << " No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
    cl::Platform default_platform = all_platforms[0];
    std::cout << "Using platform: "
              << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

    std::vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if (all_devices.size() == 0) {
        std::cout << " No devices found. Check OpenCL installation!\n";
        exit(1);
    }
    cl::Device default_device = all_devices[0];
    std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>()
              << "\n";
    return default_device;
}

GLuint util::getShader(std::string path) {
    GLint success;
    GLchar infoLog[512];

    // compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexSource = loadFile(path + ".vert");
    glShaderSource(vertexShader, 1, (const GLchar **)&vertexSource, nullptr);
    glCompileShader(vertexShader);
    // error check
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED: "
                  << path + ".vert"
                  << "\n"
                  << infoLog << std::endl;
    }

    // compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentSource = loadFile(path + ".frag");
    glShaderSource(fragmentShader, 1, (const GLchar **)&fragmentSource,
                   nullptr);
    glCompileShader(fragmentShader);
    // error check
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: "
                  << path + ".frag"
                  << "\n"
                  << infoLog << std::endl;
    }

    // create program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    // error check
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    // clean up
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    std::cout << "loaded shader: " << path << std::endl;

    return program;
}

util::MeshMetaData util::loadMesh(std::string path) {
    std::vector<Vertex> m_vertices;
    std::vector<GLuint> m_indices;
    std::ifstream obj(path);
    if (!obj.is_open()) {
        throw std::runtime_error("Couldn't open " + path);
    }

    std::vector<glm::vec3> v;
    std::vector<glm::vec2> vt;
    std::vector<glm::vec3> vn;
    std::vector<int> f; // position, texture, normal
    std::string line;
    while (!getline(obj, line).eof()) {
        if (line.find("v ") == 0) {
            line = line.substr(2);
            std::size_t delimiter1 = line.find(" ");
            std::size_t delimiter2 = line.find_last_of(" ");
            std::string x = line.substr(0, delimiter1);
            std::string y =
                line.substr(delimiter1 + 1, delimiter2 - delimiter1);
            std::string z = line.substr(delimiter2);
            v.push_back({std::stof(x), std::stof(y), std::stof(z)});
        } else if (line.find("vt ") == 0) {
            line = line.substr(3);
            std::size_t delimiter1 = line.find(" ");
            std::string x = line.substr(0, delimiter1);
            std::string y = line.substr(delimiter1 + 1);
            vt.push_back({std::stof(x), std::stof(y)});
        } else if (line.find("vn ") == 0) {
            line = line.substr(3);
            std::size_t delimiter1 = line.find(" ");
            std::size_t delimiter2 = line.find_last_of(" ");
            std::string x = line.substr(0, delimiter1);
            std::string y =
                line.substr(delimiter1 + 1, delimiter2 - delimiter1);
            std::string z = line.substr(delimiter2);
            vn.push_back({std::stof(x), std::stof(y), std::stof(z)});
        } else if (line.find("f ") == 0) {
            line = line.substr(2);
            auto count = std::count(line.begin(), line.end(), '/');
            if (line.find("//") != std::string::npos) { // vertex and normal
                // f v/t/n v/t/n ...

                int i = 0;
                std::size_t pos = 0;
                while (pos != std::string::npos) {
                    pos = line.find(' ');
                    std::string tmp;
                    if (pos == std::string::npos) {
                        tmp = line;
                    } else {
                        tmp = line.substr(0, pos);
                        line = line.substr(pos + 1);
                    }

                    std::size_t delimiter1 = tmp.find("//");
                    std::string v = tmp.substr(0, delimiter1);
                    std::string n = tmp.substr(delimiter1 + 2);
                    f.push_back(std::stoi(v));
                    f.push_back(-1); // no texture
                    f.push_back(std::stoi(n));

                    i++;
                }
                if (i != 3) {
                    throw std::runtime_error("Mesh is not triangulated " +
                                             path);
                }

            } else if (count == 6) { // vertex, texture, normal
                int i = 0;
                std::size_t pos = 0;
                while (pos != std::string::npos) {
                    pos = line.find_first_of(' ');
                    std::string tmp;
                    if (pos == std::string::npos) {
                        tmp = line;
                    } else {
                        tmp = line.substr(0, pos);
                        line = line.substr(pos + 1);
                    }

                    std::size_t delimiter1 = tmp.find("/");
                    std::size_t delimiter2 = tmp.find_last_of("/");
                    std::string v = tmp.substr(0, delimiter1);
                    std::string t =
                        tmp.substr(delimiter1 + 1, delimiter2 - delimiter1 - 1);
                    std::string n = tmp.substr(delimiter2 + 1);
                    f.push_back(std::stoi(v));
                    f.push_back(std::stoi(t));
                    f.push_back(std::stoi(n));

                    i++;
                }

            } else {
                obj.close();
                throw std::runtime_error(
                    "Mesh " + path +
                    "doesn't contain normals or is not triangulated");
            }
        }
    }

    obj.close();

    // create mesh
    if (f.size() % 9 != 0) { // 9 ints per face/triangle
        throw std::runtime_error("Mesh " + path +
                                 " has insufficient amount of faces");
    }

    for (unsigned int i = 0; i < f.size(); i += 3) {
        glm::vec3 position = v[f[i] - 1];
        glm::vec2 texture;
        if (f[i + 1] == -1) {
            texture = {0.0f, 0.0f};
        } else {
            texture = vt[f[i + 1] - 1];
        }
        glm::vec3 normal = vn[f[i + 2] - 1];

        Vertex vertex = {position, normal,
                         glm::vec2(texture.x, texture.y * -1)};
        int pos = -1;
        int x = 0;
        // really inefficient, specially for large meshes
        for (auto m_vertex : m_vertices) {
            if (vertex.position == m_vertex.position &&
                vertex.normal == m_vertex.normal &&
                vertex.texCoords == m_vertex.texCoords) {
                pos = x;
                break;
            }
            x++;
        }

        if (pos == -1) {
            m_vertices.push_back(vertex);
            m_indices.push_back(
                static_cast<unsigned int>(m_vertices.size() - 1));
        } else {
            m_indices.push_back(pos);
        }
    }

    MeshMetaData m_mData;

    glGenVertexArrays(1, &m_mData.vao);
    glGenBuffers(1, &m_mData.vbo);
    glGenBuffers(1, &m_mData.ibo);

    glBindVertexArray(m_mData.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_mData.vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex),
                 m_vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mData.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint),
                 m_indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid *)0);
    glEnableVertexAttribArray(1); // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid *)(sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(2); // texCoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid *)(sizeof(GLfloat) * 6));

    glBindVertexArray(0);

    m_mData.indicesCount = m_indices.size();

    return m_mData;
}

void util::freeMesh(MeshMetaData mesh) {
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
}
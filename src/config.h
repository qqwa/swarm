#include "profiler.h"
#include <cpptoml.h>
#include <glm/glm.hpp>

#include "transform.h"

class Config {
  private:
    std::shared_ptr<cpptoml::table> m_config;
    int steps;
    std::vector<std::vector<float>> camera;
    std::vector<std::vector<float>> track_point;
    std::vector<std::vector<float>> wind;

  public:
    int width;
    int height;
    int random_seed;
    float tick;
    int swarm_size;
    glm::vec3 swarm_start;
    float swarm_initial_spread;
    float sphere_size;
    float swarm_speed;
    float swarm_weight_neighbors;
    float swarm_weight_collision;
    float swarm_weight_track_point;
    float swarm_weight_swarm_center;
    float gravitation;
    std::vector<glm::vec3> spheres;

    Profiler render;
    Profiler update;
    Profiler update_neighbors_cpu;
    Profiler update_neighbors_incremental_cpu;
    Profiler update_swarm_cpu;
    Profiler update_neighbors_gpu;
    Profiler update_neighbors_incremental_gpu;
    Profiler update_swarm_gpu;

  public:
    Config(std::string path);
    bool debug(std::string key) const;

    glm::vec3 get_track_point(int tick);
    glm::vec3 get_camera_pos(int tick);
    float get_camera_pitch(int tick);
    float get_camera_yaw(int tick);
    glm::vec3 get_wind_dir(int tick);
    float get_wind_strength(int tick);
};

inline Config *config;
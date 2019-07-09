#include "config.h"
#include <iostream>

Config::Config(std::string path)
    : render(2500, "Render"), update(2500, "Update"),
      update_neighbors_cpu(2500, "Update Neighbor CPU"),
      update_swarm_cpu(2500, "Update Swarm CPU"),
      update_neighbors_incremental_cpu(2500, "Update Swarm Neighbor Inc CPU"),
      update_neighbors_gpu(2500, "Update Neighbor GPU"),
      update_swarm_gpu(2500, "Update Swarm GPU"),
      update_neighbors_incremental_gpu(2500, "Update Swarm Neighbor Inc GPU") {
    m_config = cpptoml::parse_file(path);
    std::cout << "loading config " << path << " " << m_config << "..."
              << std::endl;
    // saving everything expect debug table in class members

    // setings
    auto val = m_config->get_qualified_as<int>("settings.width");
    if (val) {
        width = *val;
    } else {
        throw std::runtime_error("config error settings.width");
    }

    val = m_config->get_qualified_as<int>("settings.height");
    if (val) {
        height = *val;
    } else {
        throw std::runtime_error("config error settings.height");
    }

    val = m_config->get_qualified_as<int>("settings.random_seed");
    if (val) {
        random_seed = *val;
    } else {
        throw std::runtime_error("config error random_seed");
    }

    // swarm
    val = m_config->get_qualified_as<int>("swarm.size");
    if (val) {
        swarm_size = *val;
    } else {
        throw std::runtime_error("config error replaceme");
    }

    auto arr = m_config->get_array_qualified("swarm.start");
    if (arr->is_array()) {
        auto val = arr->get_array_of<double>();
        if (val) {
            swarm_start = glm::vec3(val->at(0), val->at(1), val->at(2));
        } else {
            throw std::runtime_error("config error swarm.start");
        }
    } else {
        throw std::runtime_error("config error swarm.start");
    }

    auto val_d = m_config->get_qualified_as<double>("swarm.speed");
    if (val_d) {
        swarm_speed = *val_d;
    } else {
        throw std::runtime_error("config error swarm.speed");
    }

    val_d = m_config->get_qualified_as<double>("settings.sphere_size");
    if (val_d) {
        sphere_size = *val_d;
    } else {
        throw std::runtime_error("config error settings.sphere_size");
    }

    val_d = m_config->get_qualified_as<double>("swarm.initial_spread");
    if (val_d) {
        swarm_initial_spread = *val_d;
    } else {
        throw std::runtime_error("config error swarm.initial_spread");
    }

    val_d = m_config->get_qualified_as<double>("settings.tick");
    if (val_d) {
        tick = *val_d;
    } else {
        throw std::runtime_error("config error settings.tick");
    }

    val_d = m_config->get_qualified_as<double>("swarm.weight_neighbors");
    if (val_d) {
        swarm_weight_neighbors = *val_d;
    } else {
        throw std::runtime_error("config error weight_neighbors");
    }

    val_d = m_config->get_qualified_as<double>("swarm.weight_collision");
    if (val_d) {
        swarm_weight_collision = *val_d;
    } else {
        throw std::runtime_error("config error swarm.weight_collision");
    }

    val_d = m_config->get_qualified_as<double>("swarm.weight_track_point");
    if (val_d) {
        swarm_weight_track_point = *val_d;
    } else {
        throw std::runtime_error("config error swarm.weight_track_point");
    }

    val_d = m_config->get_qualified_as<double>("swarm.weight_swarm_center");
    if (val_d) {
        swarm_weight_swarm_center = *val_d;
    } else {
        throw std::runtime_error("config error swarm.weight_swarm_center");
    }

    val_d = m_config->get_qualified_as<double>("environment.gravitation");
    if (val_d) {
        gravitation = *val_d;
    } else {
        throw std::runtime_error("config error environment.gravitation");
    }

    val = m_config->get_qualified_as<int>("interpolation.steps");
    if (val) {
        steps = *val;
    } else {
        throw std::runtime_error("config error interpolation.steps");
    }

    auto n_arr = m_config->get_qualified_array_of<cpptoml::array>(
        "interpolation.track_point");
    if (n_arr) {
        for (auto vec : *n_arr) {
            auto val = vec->get_array_of<double>();
            track_point.push_back(
                {(float)val->at(0), (float)val->at(1), (float)val->at(2)});
        }
    } else {
        throw std::runtime_error("config error interpolation.track_point");
    }

    n_arr =
        m_config->get_qualified_array_of<cpptoml::array>("interpolation.enemy");
    if (n_arr) {
        for (auto vec : *n_arr) {
            auto val = vec->get_array_of<double>();
            enemy.push_back({(float)val->at(0), (float)val->at(1),
                             (float)val->at(2), (float)val->at(3)});
        }
    } else {
        throw std::runtime_error("config error interpolation.enemy");
    }

    n_arr =
        m_config->get_qualified_array_of<cpptoml::array>("interpolation.wind");
    if (n_arr) {
        for (auto vec : *n_arr) {
            auto val = vec->get_array_of<double>();
            wind.push_back({(float)val->at(0), (float)val->at(1),
                            (float)val->at(2), (float)val->at(3)});
        }
    } else {
        throw std::runtime_error("config error interpolation.wind");
    }

    n_arr = m_config->get_qualified_array_of<cpptoml::array>(
        "interpolation.camera");
    if (n_arr) {
        for (auto vec : *n_arr) {
            auto val = vec->get_array_of<double>();
            camera.push_back({(float)val->at(0), (float)val->at(1),
                              (float)val->at(2), (float)val->at(3),
                              (float)val->at(4)});
        }
    } else {
        throw std::runtime_error("config error interpolation.camera");
    }

    std::cout << "finished loading config" << std::endl;
}

bool Config::debug(std::string key) const {
    auto k = "debug." + key;
    auto val = m_config->get_qualified_as<bool>(k);
    if (val) {
        return *val;
    } else {
        std::cerr << "Tried to get config \"" << k << "\" which does not exist"
                  << std::endl;
        return false;
    }
}

glm::vec3 Config::get_track_point(int tick) {
    auto n0 = tick / steps;
    auto n1 = n0 + 1;
    auto ratio = (float)(tick % steps) / steps;

    if (n1 < track_point.size()) {
        auto val0 = track_point[n0];
        auto val1 = track_point[n1];

        return glm::vec3((1 - ratio) * val0[0] + ratio * val1[0],
                         (1 - ratio) * val0[1] + ratio * val1[1],
                         (1 - ratio) * val0[2] + ratio * val1[2]);

    } else {
        auto val = track_point[track_point.size() - 1];
        return glm::vec3(val[0], val[1], val[2]);
    }
}

glm::vec3 Config::get_camera_pos(int tick) {
    auto n0 = tick / steps;
    auto n1 = n0 + 1;
    auto ratio = (float)(tick % steps) / steps;

    if (n1 < camera.size()) {
        auto val0 = camera[n0];
        auto val1 = camera[n1];

        return glm::vec3((1 - ratio) * val0[0] + ratio * val1[0],
                         (1 - ratio) * val0[1] + ratio * val1[1],
                         (1 - ratio) * val0[2] + ratio * val1[2]);

    } else {
        auto val = camera[camera.size() - 1];
        return glm::vec3(val[0], val[1], val[2]);
    }
}

float Config::get_camera_pitch(int tick) {
    auto n0 = tick / steps;
    auto n1 = n0 + 1;
    auto ratio = (float)(tick % steps) / steps;

    if (n1 < camera.size()) {
        auto val0 = camera[n0];
        auto val1 = camera[n1];

        return (1 - ratio) * val0[3] + ratio * val1[3];

    } else {
        auto val = camera[camera.size() - 1];
        return val[3];
    }
}

float Config::get_camera_yaw(int tick) {
    auto n0 = tick / steps;
    auto n1 = n0 + 1;
    auto ratio = (float)(tick % steps) / steps;

    if (n1 < camera.size()) {
        auto val0 = camera[n0];
        auto val1 = camera[n1];

        return (1 - ratio) * val0[4] + ratio * val1[4];

    } else {
        auto val = camera[camera.size() - 1];
        return val[4];
    }
}

glm::vec3 Config::get_wind_dir(int tick) {
    auto n0 = tick / steps;
    auto n1 = n0 + 1;
    auto ratio = (float)(tick % steps) / steps;

    if (n1 < wind.size()) {
        auto val0 = wind[n0];
        auto val1 = wind[n1];

        return glm::vec3((1 - ratio) * val0[0] + ratio * val1[0],
                         (1 - ratio) * val0[1] + ratio * val1[1],
                         (1 - ratio) * val0[2] + ratio * val1[2]);

    } else {
        auto val = wind[wind.size() - 1];
        return glm::vec3(val[0], val[1], val[2]);
    }
}

float Config::get_wind_strength(int tick) {
    auto n0 = tick / steps;
    auto n1 = n0 + 1;
    auto ratio = (float)(tick % steps) / steps;

    if (n1 < wind.size()) {
        auto val0 = wind[n0];
        auto val1 = wind[n1];

        return (1 - ratio) * val0[3] + ratio * val1[3];

    } else {
        auto val = wind[wind.size() - 1];
        return val[3];
    }
}

glm::vec3 Config::get_enemy_pos(int tick) {
    auto n0 = tick / steps;
    auto n1 = n0 + 1;
    auto ratio = (float)(tick % steps) / steps;

    if (n1 < enemy.size()) {
        auto val0 = enemy[n0];
        auto val1 = enemy[n1];

        return glm::vec3((1 - ratio) * val0[0] + ratio * val1[0],
                         (1 - ratio) * val0[1] + ratio * val1[1],
                         (1 - ratio) * val0[2] + ratio * val1[2]);

    } else {
        auto val = enemy[enemy.size() - 1];
        return glm::vec3(val[0], val[1], val[2]);
    }
}

float Config::get_enemy_scale(int tick) {
    auto n0 = tick / steps;
    auto n1 = n0 + 1;
    auto ratio = (float)(tick % steps) / steps;

    if (n1 < enemy.size()) {
        auto val0 = enemy[n0];
        auto val1 = enemy[n1];

        return (1 - ratio) * val0[3] + ratio * val1[3];

    } else {
        auto val = enemy[enemy.size() - 1];
        return val[3];
    }
}
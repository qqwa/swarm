#pragma once

#include <chrono>
#include <queue>
#include <string>
#include <vector>

/**
 * @brief Profiler can be used tu measure the min, max and avg time of a
 * execution path
 */
class Profiler {
  std::vector<std::chrono::duration<float>> m_rawTime;
  std::string m_msg;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
  int m_size;

public:
  Profiler(int buffersize, std::string output);

  ~Profiler();

  /**
   * @brief Start starts the measurment of a new entry in the buffer
   */
  void Start();

  /**
   * @brief Stop stops a currently running measurment and add the measuered time
   * to the buffer
   */
  void Stop();

  /**
   * @brief Outputs the avg, min and max time in a nice formated way, with a
   * prefix from the prifler
   */
  friend std::ostream &operator<<(std::ostream &stream,
                                  const Profiler &profiler);
};


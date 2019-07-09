#include "profiler.h"
#include <iomanip>
#include <iostream>

Profiler::Profiler(int buffersize, std::string output)
    : m_size(buffersize), m_msg(output) {}

Profiler::~Profiler() {}

void Profiler::Start() { m_start = std::chrono::high_resolution_clock::now(); }

void Profiler::Stop() {
    auto stop = std::chrono::high_resolution_clock::now();
    m_rawTime.push_back(stop - m_start);
    if (m_size < m_rawTime.size()) {
        m_rawTime.erase(m_rawTime.begin());
    }
}

std::ostream &operator<<(std::ostream &stream, const Profiler &profiler) {
    if (profiler.m_rawTime.size() == 0) {
        return stream << profiler.m_msg << " No Data collected";
    }
    std::chrono::duration<float> avgTime(0);
    std::chrono::duration<float> minTime = profiler.m_rawTime[0];
    std::chrono::duration<float> maxTime = profiler.m_rawTime[0];
    for (auto &entry : profiler.m_rawTime) {
        avgTime += entry;
    }
    for (auto &entry : profiler.m_rawTime) {
        if (entry < minTime) {
            minTime = entry;
        }
    }
    for (auto &entry : profiler.m_rawTime) {
        if (maxTime < entry) {
            maxTime = entry;
        }
    }
    avgTime /= profiler.m_rawTime.size();

    return stream << profiler.m_msg << "(" << profiler.m_rawTime.size()
                  << "):" << std::setprecision(2) << std::fixed << "[Avg Time:"
                  << std::chrono::duration_cast<
                         std::chrono::duration<float, std::milli>>(avgTime)
                         .count()
                  << "ms"
                  << "], [Min Time:"
                  << std::chrono::duration_cast<
                         std::chrono::duration<float, std::milli>>(minTime)
                         .count()
                  << "ms"
                  << "], [Max Time:"
                  << std::chrono::duration_cast<
                         std::chrono::duration<float, std::milli>>(maxTime)
                         .count()
                  << "ms"
                  << "]";
}
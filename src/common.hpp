#pragma once

#include <utility>
#include <random>
#include <cmath>

struct pair_hash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ std::hash<int>()(p.second << 1);
    }
};

inline float generateRandomFloat(float min, float max) {
    std::random_device rd;                               // Seed source
    std::mt19937 gen(rd());                              // Mersenne Twister engine
    std::uniform_real_distribution<float> dis(min, max); // Range [min, max)
    return dis(gen);
}

inline uint32_t make_grid_key(int row, int col) {
    return (static_cast<uint32_t>(row) << 16) | static_cast<uint32_t>(col);
}



#include <vector>
#include <iostream>

#include "map.hpp"

namespace Maze {
    extern std::vector<std::pair<int, int>> directions_sec_3;
    extern std::vector<std::pair<int, int>> directions_sec_2;
    extern std::vector<std::pair<int, int>> directions_sec_1;

    extern std::vector<std::pair<int, int>> path;
    extern int pathway;

    void shuffle_directions(std::vector<std::pair<int, int>>& directions);
    void generate_maze(std::vector<int>& terrainMap, int start_row, int start_col, std::string type);
    bool is_walkable(int gridValue);
    bool find_path(const std::vector<int>& terrainMap, int sx, int sy, int gx, int gy);
    void punch_sector_exits(
        std::vector<int>& terrainMap,
        int halfMapSize,
        int ringRadius,
        int innerPath,
        int outerPath,
        int numExits,
        float angleOffset = 0.0f,
        bool doorsInMiddle = false
    );
}



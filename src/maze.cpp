#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <random>
#include <queue>
#include <utility>
#include <unordered_map>
#include <cmath>

#include "map.hpp"
#include "common.hpp"

namespace Maze
{
    // Directions: up, down, left, right
    // All directions must use the SAME step size so the DFS grid nodes align.
    std::vector<std::pair<int, int>> directions_sec_3 = {
        {-4, 0}, {4, 0}, {0, -4}, {0, 4}};

    // FIX: was {-4,0},{2,0},{0,-4},{0,2} — asymmetric steps meant the grid
    // nodes never shared the same lattice, creating disconnected islands.
    std::vector<std::pair<int, int>> directions_sec_2 = {
        {-4, 0}, {4, 0}, {0, -4}, {0, 4}};

    std::vector<std::pair<int, int>> directions_sec_1 = {
        {-3, 0}, {3, 0}, {0, -3}, {0, 3}};

    std::vector<std::pair<int, int>> path;
    int pathway = Map::ERROR_CUBE;

    void shuffle_directions(std::vector<std::pair<int, int>> &directions)
    {
        static std::random_device rd;
        static std::mt19937 g(rd());
        std::shuffle(directions.begin(), directions.end(), g);
    }

    // Recursive maze generation using DFS
    void generate_maze(std::vector<int> &terrainMap, int start_row, int start_col, std::string type)
    {
        std::vector<std::pair<int, int>> directions;
        int allowed_number;

        if (type == "one")
        {
            directions = directions_sec_1;
            allowed_number = Map::SECTOR_1_WALL_VAL;
            pathway = Map::SECTOR_1_PATHWAY;
        }
        else if (type == "two")
        {
            directions = directions_sec_2;
            allowed_number = Map::SECTOR_2_WALL_VAL;
            pathway = Map::SECTOR_2_PATHWAY;
        }
        else if (type == "three")
        {
            directions = directions_sec_3;
            allowed_number = Map::SECTOR_3_WALL_VAL;
            pathway = Map::SECTOR_3_PATHWAY;
        }

        shuffle_directions(directions);

        for (const auto &dir : directions)
        {
            int nx = start_col + dir.first;
            int ny = start_row + dir.second;

            // Bounds check BEFORE map access to avoid out-of-bounds read.
            if (nx <= 0 || ny <= 0 || nx >= MAP_SIDE_LENGTH - 1 || ny >= MAP_SIDE_LENGTH - 1)
                continue;

            if (terrainMap[get_map_index(ny, nx)] != allowed_number)
            {
                continue;
            }

            terrainMap[get_map_index(ny, nx)] = pathway;

            if (type == "one")
            {
                // Carve the 2x2 destination block
                terrainMap[get_map_index(ny + 1, nx)] = pathway;
                terrainMap[get_map_index(ny, nx + 1)] = pathway;
                terrainMap[get_map_index(ny + 1, nx + 1)] = pathway;

                // Carve the bridge between the 2x2 blocks
                if (dir.first != 0)
                {
                    int step = (dir.first > 0) ? 1 : -1;
                    terrainMap[get_map_index(start_row, start_col + step)] = pathway;
                    terrainMap[get_map_index(start_row, start_col + (step * 2))] = pathway;
                    terrainMap[get_map_index(start_row + 1, start_col + step)] = pathway;
                    terrainMap[get_map_index(start_row + 1, start_col + (step * 2))] = pathway;
                }
                else if (dir.second != 0)
                {
                    int step = (dir.second > 0) ? 1 : -1;
                    terrainMap[get_map_index(start_row + step, start_col)] = pathway;
                    terrainMap[get_map_index(start_row + (step * 2), start_col)] = pathway;
                    terrainMap[get_map_index(start_row + step, start_col + 1)] = pathway;
                    terrainMap[get_map_index(start_row + (step * 2), start_col + 1)] = pathway;
                }
            }

            else if (type == "two" || type == "three")
            {
                // FIX: normalize direction to unit steps so we carve each tile
                // BETWEEN start and destination, not beyond it.
                //
                // Old code: dir.first + step (e.g. 4+1=5, 4+2=6 — past the dest!)
                // New code: unit_step * step  (e.g. 1,2,3 — the gap in-between)
                int unit_x = (dir.first != 0) ? (dir.first > 0 ? 1 : -1) : 0;
                int unit_y = (dir.second != 0) ? (dir.second > 0 ? 1 : -1) : 0;
                int dist = std::abs(dir.first != 0 ? dir.first : dir.second);

                for (int step = 1; step < dist; ++step)
                {
                    int cy = start_row + unit_y * step;
                    int cx = start_col + unit_x * step;
                    if (cx <= 0 || cy <= 0 || cx >= MAP_SIDE_LENGTH - 1 || cy >= MAP_SIDE_LENGTH - 1)
                        break;

                    // FIX: was `return` — that killed the entire recursive call,
                    // stopping exploration of all remaining directions.
                    // `break` correctly just stops carving this one corridor.
                    if (type == "three" && terrainMap[get_map_index(cy, cx)] != allowed_number)
                        break;

                    terrainMap[get_map_index(cy, cx)] = pathway;
                }
            }

            generate_maze(terrainMap, ny, nx, type);
        }
    }

    // Punch guaranteed walkable exits through an INGROWN_WALL ring.
    // Call this AFTER generating both maze sectors that share the ring.
    //
    //   halfMAP_SIDE_LENGTH  — center of the map
    //   ringRadius   — radius of the INGROWN_WALL ring to punch through
    //   innerPath   — tile value to write on the inner side of the gap
    //   outerPath   — tile value to write on the outer side of the gap
    //   numExits    — how many exits to punch (4 = N/S/E/W, 8 = + diagonals)
    //   angleOffset — rotate all exits by this many radians (0 = East = right)
    //
    // The exits are aligned to cardinal directions (0, π/2, π, 3π/2) by default,
    // which matches the existing sector-1 pathway corridors from generate_ground.
    void punch_sector_exits(
        std::vector<int> &terrainMap,
        int halfMAP_SIDE_LENGTH,
        int ringRadius,
        int innerPath,
        int outerPath,
        int numExits = 4,
        float angleOffset = 0.0f,
        bool doorsInMiddle = false)
    {

        const float TWOPI = 2.0f * 3.14159265f;

        for (int i = 0; i < numExits; ++i)
        {
            float angle = angleOffset + (TWOPI * i) / numExits;

            // Radial direction (inward → outward through the wall)
            float rdx = std::cos(angle);
            float rdy = std::sin(angle);

            // Tangential direction (perpendicular to radius = the "width" axis)
            float tdx = -std::sin(angle);
            float tdy = std::cos(angle);

            int exitWidth = 4;
            int halfW = exitWidth / 2;

            // Walk radially from inner edge to outer edge of the ring
            for (int r = ringRadius - 3; r <= ringRadius + 3; ++r)
            {
                int tile = (r < ringRadius) ? innerPath : outerPath;

                // Carve the full width at this depth
                for (int t = -halfW; t <= halfW; ++t)
                {
                    int x = (MAP_SIDE_LENGTH * 0.5) + static_cast<int>(std::round(r * rdx + t * tdx));
                    int y = (MAP_SIDE_LENGTH * 0.5) + static_cast<int>(std::round(r * rdy + t * tdy));

                    if (doorsInMiddle && r == ringRadius)
                    {
                        // terrainMap[get_map_index(x, y)] = Map::BLUE_CUBE;
                        if (x == (MAP_SIDE_LENGTH * 0.5) || x - 1 == (MAP_SIDE_LENGTH * 0.5))
                        {
                            terrainMap[get_map_index(x, y)] = Map::MAZE_WE_DOOR;
                        }
                        if (y == (MAP_SIDE_LENGTH * 0.5) || y - 1 == (MAP_SIDE_LENGTH * 0.5))
                        {
                            terrainMap[get_map_index(x, y)] = Map::MAZE_NS_DOOR;
                        }
                        continue;
                    }

                    if (x > 0 && x < MAP_SIDE_LENGTH - 1 && y > 0 && y < MAP_SIDE_LENGTH - 1)
                    {
                        terrainMap[get_map_index(x, y)] = tile;
                    }
                }
            }
        }
    }

    bool is_walkable(int gridValue)
    {
        return wallValues.find(gridValue) == wallValues.end();
    }

    // A* Pathfinding from (sx, sy) to (gx, gy) with 8-way movement
    bool find_path(const std::vector<int> &terrainMap, int sx, int sy, int gx, int gy)
    {
        path.clear();

        using pii = std::pair<int, int>;
        std::unordered_map<pii, pii, pair_hash> came_from;

        const double INF = 1e12;
        static double g_score[MAP_SIDE_LENGTH][MAP_SIDE_LENGTH];
        for (int y = 0; y < MAP_SIDE_LENGTH; ++y)
            for (int x = 0; x < MAP_SIDE_LENGTH; ++x)
                g_score[y][x] = INF;

        auto heuristic = [&](int x, int y)
        {
            double dx = double(gx - x);
            double dy = double(gy - y);
            return std::sqrt(dx * dx + dy * dy);
        };

        int dxs[8] = {-1, 1, 0, 0, -1, -1, 1, 1};
        int dys[8] = {0, 0, -1, 1, -1, 1, -1, 1};

        using PQElem = std::pair<double, pii>;
        std::priority_queue<PQElem, std::vector<PQElem>, std::greater<PQElem>> open;

        g_score[sy][sx] = 0.0;
        open.push({heuristic(sx, sy), {sx, sy}});

        while (!open.empty())
        {
            auto [f, coord] = open.top();
            open.pop();
            int x = coord.first, y = coord.second;

            if (x == gx && y == gy)
            {
                pii curr = {gx, gy};
                while (!(curr.first == sx && curr.second == sy))
                {
                    path.push_back(curr);
                    curr = came_from[curr];
                }
                path.push_back({sx, sy});
                std::reverse(path.begin(), path.end());
                return true;
            }

            for (int i = 0; i < 8; ++i)
            {
                int nx = x + dxs[i];
                int ny = y + dys[i];
                if (nx < 0 || ny < 0 || nx >= MAP_SIDE_LENGTH || ny >= MAP_SIDE_LENGTH)
                    continue;
                if (!is_walkable(terrainMap[get_map_index(ny, nx)]))
                    continue;

                double move_cost = (dxs[i] == 0 || dys[i] == 0) ? 1.0 : 1.41421356237;
                double tentative_g = g_score[y][x] + move_cost;

                if (tentative_g < g_score[ny][nx])
                {
                    came_from[{nx, ny}] = {x, y};
                    g_score[ny][nx] = tentative_g;
                    open.push({tentative_g + heuristic(nx, ny), {nx, ny}});
                }
            }
        }

        return false;
    }
}
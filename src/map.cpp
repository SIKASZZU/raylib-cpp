
#include <iostream>
#include <cmath>
#include <algorithm>
#include <utility>
#include <set>
#include <random>
#include "raymath.h"

#include "map.hpp"
#include "common.hpp"
#include "maze.hpp"
#include "game_config.hpp"

std::vector<int> terrainMap((MAP_SIDE_LENGTH * MAP_SIDE_LENGTH), 0);
std::unordered_set<uint32_t> sector3Cutouts;

std::set<std::pair<int, int>> &getS3PatchLocations()
{
    static std::set<std::pair<int, int>> s3PatchLocations;
    return s3PatchLocations;
}

namespace MapGenerator
{

    /* variables for just map generation */
    std::set<std::pair<int, int>> voidPossibleLocations;
    int gladeRadius = (MAP_SIDE_LENGTH / 10) >= 10 ? 10 : MAP_SIDE_LENGTH / 10; // if MAP_SIDE_LENGTH / 10 > 10; hard cap to 10.

    int mazeSize = MAP_SIDE_LENGTH / 2 - 4;
    int halfMapSize = MAP_SIDE_LENGTH / 2;
    int maxMazeSize = 130;

    int mazeInnerRadius = gladeRadius + 1;
    int mazeOuterRadius = std::min(mazeSize, maxMazeSize);

    int mazeThirdSector = mazeOuterRadius - (mazeOuterRadius / 3);
    int mazeSecondSector = (mazeOuterRadius - mazeInnerRadius) / 2.5f;

    const int numSectors = 8;

    // including wall for each side
    const int cliffWidth = 4;
    const int cliffWidthHalf{cliffWidth / 2};
    const int tunnelWidth = 7;

    void init()
    {
        terrainMap.assign(MAP_SIDE_LENGTH * MAP_SIDE_LENGTH, Map::EMPTY);
        // diagonalGrids.clear();
        getS3PatchLocations().clear();
        sector3Cutouts.clear();
        // SECTOR_3_REMOVE_EXTRA_THICKNESS.clear();
        voidPossibleLocations.clear();

        generate_ground();
        generate_mazes();
        generate_glade(terrainMap);
        // generate_cliffs();
        // generate_tunnels();
    }

    void generate_ground()
    {
        float maxDistance = std::sqrt((MAP_SIDE_LENGTH * 0.5) * (MAP_SIDE_LENGTH * 0.5) + (MAP_SIDE_LENGTH * 0.5) * (MAP_SIDE_LENGTH * 0.5));
        for (int y = 0; y < MAP_SIDE_LENGTH; y++)
        {
            for (int x = 0; x < MAP_SIDE_LENGTH; x++)
            {

                int dx = static_cast<int>(std::abs(x - (MAP_SIDE_LENGTH * 0.5)));
                int dy = static_cast<int>(std::abs(y - (MAP_SIDE_LENGTH * 0.5)));

                float distance_sq = dx * dx + dy * dy;
                int distance = std::sqrt(distance_sq);

                float angle = std::atan2(dy, dx);
                if (angle < 0)
                    angle += 2 * PI;
                float sector_angle = 2 * PI / numSectors;

                float norm_dist = distance / maxDistance;
                float landChance = 1.0f - norm_dist;
                landChance += ((rand() % 100) / 100.0f - 0.5f) * 0.8f;
                landChance = std::clamp(landChance, 0.0f, 1.0f);

                if (distance >= mazeOuterRadius)
                {
                    if (landChance != 0.0f)
                    {
                        terrainMap[get_map_index(x, y)] = Map::GROUND_CUBE;
                    }
                    else
                    {
                        rand() % 2 == 1 ? terrainMap[get_map_index(x, y)] = Map::TREE_TRUNK : terrainMap[get_map_index(x, y)] = Map::TREE;
                    }
                }

                // maze ala full maze_ground, section overwritib oma enda dataga.
                if (distance >= mazeInnerRadius && distance <= mazeOuterRadius)
                {
                    terrainMap[get_map_index(x, y)] = Map::SECTOR_1_PATHWAY;
                }
                int interlapBuffer = 3;
                if (distance >= mazeInnerRadius && distance <= mazeSecondSector - interlapBuffer)
                {
                    terrainMap[get_map_index(x, y)] = Map::SECTOR_1_WALL_VAL;
                }
                else if (distance >= mazeSecondSector && distance <= mazeThirdSector)
                {
                    terrainMap[get_map_index(x, y)] = Map::SECTOR_2_WALL_VAL;
                }
                else if (distance >= mazeThirdSector && distance <= mazeOuterRadius)
                {
                    terrainMap[get_map_index(x, y)] = Map::SECTOR_3_WALL_VAL;
                }

                for (int sector = 0; sector < numSectors; ++sector)
                {
                    float wallAngle = sector * sector_angle;
                    // normalize to [-PI, PI]
                    float delta = std::fmod(angle - wallAngle + PI, 2 * PI) - PI;

                    // mapi nurkades on suured metsad o_o,
                    if (distance >= mazeOuterRadius)
                    {

                        terrainMap[get_map_index(x, y)] = Map::EMPTY;
                        continue;

                        float sizeOfForest = 0.3f;
                        if (sector % 2 != 0 && std::fabs(delta) < sizeOfForest && landChance <= 0.3f)
                        {
                            rand() % 125 == 1 ? terrainMap[get_map_index(x, y)] = Map::TREE_TRUNK : terrainMap[get_map_index(x, y)] = Map::TREE;
                        }
                        // metsades on portaalid O_O,
                        // varsti saab neist sisse minna (❁´◡`❁)
                        if (sector % 2 != 0 && std::fabs(delta) < 0.15f && distance <= (MAP_SIDE_LENGTH * 0.5) && distance >= mazeOuterRadius)
                        {
                            voidPossibleLocations.insert({y, x});
                        }
                    }
                    // walls between sector 1/2
                    int thicknessSectionWall = 2;
                    if (distance <= mazeSecondSector && distance >= mazeSecondSector - thicknessSectionWall && std::fabs(delta) < 0.3)
                    {
                        terrainMap[get_map_index(x, y)] = Map::INGROWN_WALL_CUBE;
                    }
                }

                // walls between sector 3/outside
                // if (distance == mazeOuterRadius) {
                if (distance >= mazeOuterRadius && distance <= mazeOuterRadius)
                {
                    terrainMap[get_map_index(x, y)] = Map::INGROWN_WALL_CUBE;
                }

                // walls between sector 2/3
                if (distance <= mazeThirdSector && distance >= mazeThirdSector - 1)
                {
                    terrainMap[get_map_index(x, y)] = Map::INGROWN_WALL_CUBE;
                }

                // walls around the terrainMap, map border.
                if (y == 0 || x == 0 || y == (MAP_SIDE_LENGTH - 1) || x == (MAP_SIDE_LENGTH - 1))
                {
                    terrainMap[get_map_index(x, y)] = Map::INGROWN_WALL_CUBE;
                }

                // 4 sector walls (diagonals)
                // if (distance <= mazeOuterRadius
                //     && dx > gladeRadius + 1
                //     && dy > gladeRadius + 1) {
                //     if (y == x || x == MAP_SIDE_LENGTH - y - 1) diagonalGrids.push_back(std::make_pair(y, x));
                // }
            }
        }
    }

    void generate_mazes()
    {

        bool sector_1 = false, sector_2 = false, sector_3 = false;

        auto find_start_sectors = [&]() -> bool
        {
            for (int i = 0; i < MAP_SIDE_LENGTH; i++)
            {
                for (int j = 0; j < MAP_SIDE_LENGTH; j++)
                {
                    if (terrainMap[get_map_index(i, j)] == Map::SECTOR_1_WALL_VAL && sector_1 == false)
                    {
                        std::cout << "Generating SECTION 1: Start grid: " << i << " " << j << "\n";
                        Maze::generate_maze(terrainMap, i, j, "one");
                        sector_1 = true;
                    }
                    if (terrainMap[get_map_index(i, j)] == Map::SECTOR_2_WALL_VAL && sector_2 == false)
                    {
                        std::cout << "Generating SECTION 2: Start grid: " << i << " " << j << "\n";
                        Maze::generate_maze(terrainMap, i, j, "two");
                        sector_2 = true;
                    }
                    if (terrainMap[get_map_index(i, j)] == Map::SECTOR_3_WALL_VAL && sector_3 == false)
                    {
                        std::cout << "Generating SECTION 3: Start grid: " << i << " " << j << "\n";
                        Maze::generate_maze(terrainMap, i, j, "three");
                        sector_3 = true;
                    }
                    if (sector_1 == true && sector_2 == true && sector_3 == true)
                        return true;
                }
            }
            return false;
        };

        std::cout << "sector status: " << find_start_sectors() << "\n";

        // mod_map_sector_3();

        // N/S/E/W exits // punchouts
        Maze::punch_sector_exits(terrainMap, MAP_SIDE_LENGTH * 0.5, mazeSecondSector, Map::SECTOR_1_PATHWAY, Map::SECTOR_2_PATHWAY, 4, 0.0f);
        Maze::punch_sector_exits(terrainMap, MAP_SIDE_LENGTH * 0.5, mazeThirdSector, Map::SECTOR_2_PATHWAY, Map::SECTOR_3_PATHWAY, 4, 0.0f, true);
        Maze::punch_sector_exits(terrainMap, MAP_SIDE_LENGTH * 0.5, mazeOuterRadius, Map::SECTOR_3_PATHWAY, Map::SECTOR_3_PATHWAY, 4, 0.0f);

        // optimze_walls();
        generate_s1_clearspots();
        // diagonal_seperation_walls();
        generate_s3_patches(8);
    }

    void generate_s3_patches(int patchWidth)
    {
        float sector_angle = 2.0f * PI / numSectors;
        int radius = patchWidth / 2;

        for (int s = 0; s < numSectors; ++s)
        {
            bool found = false;
            int attempts = 0;
            while (!found && attempts < 200)
            {
                attempts++;

                // Random angle in sector s
                float angle = (s + ((rand() % 100) / 100.0f)) * sector_angle;

                // Random distance in section 3
                float dist = mazeThirdSector + ((mazeOuterRadius - mazeThirdSector) / 2);

                int cx = (MAP_SIDE_LENGTH * 0.5) + static_cast<int>(dist * std::cos(angle));
                int cy = (MAP_SIDE_LENGTH * 0.5) + static_cast<int>(dist * std::sin(angle));

                if (cx >= radius && cx < MAP_SIDE_LENGTH - radius && cy >= radius && cy < MAP_SIDE_LENGTH - radius)
                {
                    // Check if it's in a reasonable place (optional, but good for visibility)
                    // Draw diamond
                    for (int dy = -radius; dy <= radius; ++dy)
                    {
                        for (int dx = -radius; dx <= radius; ++dx)
                        {
                            if (std::abs(dx) + std::abs(dy) <= radius)
                            {
                                terrainMap[get_map_index(cx + dx, cy + dy)] = Map::BLUE_CUBE;
                                getS3PatchLocations().insert(std::make_pair(cx + dx, cy + dy));
                            }
                            if (dy == 0 && dx == 0)
                            {
                                terrainMap[get_map_index(cx + dx, cy + dy)] = Map::ERROR_CUBE;
                            }
                        }
                    }
                    found = true;
                }
            }
        }
    }

    void generate_s1_clearspots()
    {
        for (int y = 0; y < MAP_SIDE_LENGTH; y++)
        {
            for (int x = 0; x < MAP_SIDE_LENGTH; x++)
            {

                int dx = static_cast<int>(std::abs(x - (MAP_SIDE_LENGTH * 0.5)));
                int dy = static_cast<int>(std::abs(y - (MAP_SIDE_LENGTH * 0.5)));

                float distance_sq = dx * dx + dy * dy;
                int distance = std::sqrt(distance_sq);

                float angle = std::atan2(dy, dx);
                if (angle < 0)
                    angle += 2 * PI;
                float sector_angle = 2 * PI / numSectors;

                for (int sector = 0; sector < numSectors; ++sector)
                {
                    float wallAngle = sector * sector_angle;
                    // normalize to [-PI, PI]
                    float delta = std::fmod(angle - wallAngle + PI, 2 * PI) - PI;

                    // diagonaalidel mazei vahekohad (sec1)
                    if (sector % 2 != 0)
                    {
                        if (distance <= (mazeSecondSector * 0.7))
                        { //  && (distance >= mazeInnerRadius * 3.3)
                            // kontrollib section wallide thicknessi.
                            if (std::fabs(delta) < 0.5)
                            {
                                terrainMap[get_map_index(x, y)] = Map::GROUND_CUBE;
                            }
                            // water patches
                            if (std::fabs(delta) < 0.1)
                            {
                                terrainMap[get_map_index(x, y)] = Map::BLUE_CUBE;
                            }
                        }
                    }
                    // pathwayd suunas kell 12, 3, 6, 9, et player gladeist minema saaks.
                    else if (sector % 2 == 0 && distance <= (mazeSecondSector / 2) && (x == (MAP_SIDE_LENGTH * 0.5) || y == (MAP_SIDE_LENGTH * 0.5)))
                    {

                        int pathwayWidth = 3;
                        int halfWidth = pathwayWidth / 2;
                        for (int i = -halfWidth; i <= halfWidth; i++)
                        {
                            int targetX = (x == (MAP_SIDE_LENGTH * 0.5)) ? x + i : x;
                            int targetY = (y == (MAP_SIDE_LENGTH * 0.5)) ? y + i : y;
                            terrainMap[get_map_index(targetY, targetX)] = Map::SECTOR_1_PATHWAY;
                        }
                    }
                }
            }
        }
    }

    void generate_glade(std::vector<int> &terrainMap)
    {

        for (int y = 0; y < MAP_SIDE_LENGTH; y++)
        {
            for (int x = 0; x < MAP_SIDE_LENGTH; x++)
            {
                int dx = static_cast<int>(std::abs(x - (MAP_SIDE_LENGTH * 0.5)));
                int dy = static_cast<int>(std::abs(y - (MAP_SIDE_LENGTH * 0.5)));

                // Glade (square) and Ingrown walls around glade
                int thicknessGladeWall = 1;
                if (dx <= gladeRadius + thicknessGladeWall && dy <= gladeRadius + thicknessGladeWall)
                {

                    // inner glade
                    terrainMap[get_map_index(x, y)] = Map::GROUND_CUBE;

                    // walls outside of glade radius
                    if (dx == gladeRadius + 1 || dy == gladeRadius + 1)
                    {
                        if (x != (MAP_SIDE_LENGTH * 0.5) && y != (MAP_SIDE_LENGTH * 0.5))
                        {
                            terrainMap[get_map_index(x, y)] = Map::INGROWN_WALL_CUBE;
                        }

                        if (x == (MAP_SIDE_LENGTH * 0.5) || x - 1 == (MAP_SIDE_LENGTH * 0.5))
                        {
                            terrainMap[get_map_index(x, y)] = Map::MAZE_WE_DOOR;
                        }
                        if (y == (MAP_SIDE_LENGTH * 0.5) || y - 1 == (MAP_SIDE_LENGTH * 0.5))
                        {
                            terrainMap[get_map_index(x, y)] = Map::MAZE_NS_DOOR;
                        }
                    }
                }
            }
        }
    }

    void generate_voids()
    {
        std::cout << "Possible void spawnpoints in set: " << voidPossibleLocations.size() << '\n';

        const int maxVoids = 10;
        if (voidPossibleLocations.empty())
            std::cout << "Alert: No voids spawned! voidPossibleLocations.empty()";

        std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<size_t> dist(0, voidPossibleLocations.size() - 1);

        std::vector<std::pair<int, int>> locations(voidPossibleLocations.begin(), voidPossibleLocations.end());

        if (locations.empty())
        {
            std::cout << "Error turning set into vector in generate_voids. Returned.\n";
            return;
        }

        for (int i = 0; i < maxVoids; i++)
        {
            auto [y, x] = locations[dist(rng)];
            std::cout << "Assigned void pair to map data: " << y << " " << x << '\n';
            terrainMap[get_map_index(x, y)] = Map::VOID_CUBE;
        }
    }

    void generate_cliffs()
    {
        // this function is broken, because when cliffWidth is odd number, division creates half width narrower than expected cliff width
        const int wallEnum = Map::INGROWN_WALL_CUBE;
        const int pathEnum = Map::SECTOR_3_PATHWAY;

        for (int y = 0; y < MAP_SIDE_LENGTH; y++)
        {
            for (int x = 0; x < MAP_SIDE_LENGTH; x++)
            {
                int dx = std::abs(x - (MAP_SIDE_LENGTH * 0.5));
                int dy = std::abs(y - (MAP_SIDE_LENGTH * 0.5));
                int distance = std::sqrt(dx * dx + dy * dy);
                if ((x == (MAP_SIDE_LENGTH * 0.5) || y == (MAP_SIDE_LENGTH * 0.5)) && distance >= mazeOuterRadius)
                {
                    // Widen the cliff
                    for (int i = -cliffWidthHalf; i <= cliffWidthHalf; i++)
                    {

                        int changeInto = pathEnum;

                        int tx = x, ty = y;
                        if (x == (MAP_SIDE_LENGTH * 0.5))
                            tx = x + i; // Vertical cliff, widen horizontally
                        else if (y == (MAP_SIDE_LENGTH * 0.5))
                            ty = y + i; // Horizontal cliff, widen vertically

                        // cliff pathway
                        if (tx >= 0 && tx < MAP_SIDE_LENGTH && ty >= 0 && ty < MAP_SIDE_LENGTH && changeInto != wallEnum)
                        {
                            terrainMap[get_map_index(tx, ty)] = changeInto;
                        }

                        // below tx, ty are being modified!
                        // walls for cliff
                        if ((i == -cliffWidthHalf || i == cliffWidthHalf) && x == (MAP_SIDE_LENGTH * 0.5))
                        {
                            tx += (i == -cliffWidthHalf) ? -1 : 0;
                            if (tx >= 0 && tx < MAP_SIDE_LENGTH && ty >= 0 && ty < MAP_SIDE_LENGTH)
                            {
                                terrainMap[get_map_index(tx, ty)] = wallEnum;
                            }
                        }
                        else if ((i == -cliffWidthHalf || i == cliffWidthHalf) && y == (MAP_SIDE_LENGTH * 0.5))
                        {
                            ty += (i == -cliffWidthHalf) ? -1 : 0;
                            if (tx >= 0 && tx < MAP_SIDE_LENGTH && ty >= 0 && ty < MAP_SIDE_LENGTH)
                            {
                                terrainMap[get_map_index(tx, ty)] = wallEnum;
                            }
                        }
                    }
                }
            }
        }

        // doors at mazeOuterRadius
        for (int y = 0; y < MAP_SIDE_LENGTH; y++)
        {
            for (int x = 0; x < MAP_SIDE_LENGTH; x++)
            {
                int innerMax = 2 * mazeOuterRadius + ((MAP_SIDE_LENGTH - (2 * mazeOuterRadius)) / 2);
                int innerMin = ((MAP_SIDE_LENGTH - (2 * mazeOuterRadius)) / 2);

                bool onCliffIntersection = ((x >= (MAP_SIDE_LENGTH * 0.5) - cliffWidthHalf && x <= (MAP_SIDE_LENGTH * 0.5) + cliffWidthHalf && (y == innerMin || y == innerMax)) ||
                                            (y >= (MAP_SIDE_LENGTH * 0.5) - cliffWidthHalf && y <= (MAP_SIDE_LENGTH * 0.5) + cliffWidthHalf && (x == innerMin || x == innerMax)));

                if (!onCliffIntersection)
                    continue;

                if (x == (MAP_SIDE_LENGTH * 0.5) || x + 1 == (MAP_SIDE_LENGTH * 0.5))
                {
                    terrainMap[get_map_index(x, y)] = Map::MAZE_WE_DOOR;
                }
                else if (y == (MAP_SIDE_LENGTH * 0.5) || y + 1 == (MAP_SIDE_LENGTH * 0.5))
                {
                    terrainMap[get_map_index(x, y)] = Map::MAZE_NS_DOOR;
                }
                else
                {
                    terrainMap[get_map_index(x, y)] = wallEnum;
                }
            }
        }
    }

    void generate_tunnels()
    {
        const int innerMin = tunnelWidth - 1;
        const int innerMax = MAP_SIDE_LENGTH - tunnelWidth;

        for (int y = 0; y < MAP_SIDE_LENGTH; y++)
        {
            for (int x = 0; x < MAP_SIDE_LENGTH; x++)
            {

                bool inTunnel = x < tunnelWidth || x >= MAP_SIDE_LENGTH - tunnelWidth || y < tunnelWidth || y >= MAP_SIDE_LENGTH - tunnelWidth;
                if (!inTunnel)
                    continue;

                // exit on x == 0, y halfpoint, 3 tile wide
                if (y == 0 && (x == (MAP_SIDE_LENGTH * 0.5) + 1 || x == (MAP_SIDE_LENGTH * 0.5) || x == (MAP_SIDE_LENGTH * 0.5) - 1))
                {
                    terrainMap[get_map_index(x, y)] = Map::EMPTY;
                    continue;
                }

                bool outerBorder = (x == 0 || y == 0 || x == MAP_SIDE_LENGTH - 1 || y == MAP_SIDE_LENGTH - 1);
                bool innerWall = ((x == innerMin && (y >= tunnelWidth - 1 && y < MAP_SIDE_LENGTH - tunnelWidth + 1)) || (y == innerMin && (x >= tunnelWidth - 1 && x < MAP_SIDE_LENGTH - tunnelWidth + 1)) || (x == innerMax && (y >= tunnelWidth && y < MAP_SIDE_LENGTH - tunnelWidth + 1)) || (y == innerMax && (x >= tunnelWidth && x < MAP_SIDE_LENGTH - tunnelWidth + 1)));

                bool onCliffIntersection = (((x >= (MAP_SIDE_LENGTH * 0.5) - cliffWidthHalf && x <= (MAP_SIDE_LENGTH * 0.5) + cliffWidthHalf && (y == innerMin || y == innerMax)) || (y >= (MAP_SIDE_LENGTH * 0.5) - cliffWidthHalf && y <= (MAP_SIDE_LENGTH * 0.5) + cliffWidthHalf && (x == innerMin || x == innerMax))));

                int changeInto = Map::TUNNEL_PATHWAY;
                if (innerWall)
                    changeInto = Map::TUNNEL_WALL;
                if (onCliffIntersection)
                {

                    if (x == (MAP_SIDE_LENGTH * 0.5) || x + 1 == (MAP_SIDE_LENGTH * 0.5))
                    {
                        changeInto = Map::MAZE_WE_DOOR;
                    }
                    if (y == (MAP_SIDE_LENGTH * 0.5) || y + 1 == (MAP_SIDE_LENGTH * 0.5))
                    {
                        changeInto = Map::MAZE_NS_DOOR;
                    }
                }
                if (outerBorder)
                    changeInto = Map::TUNNEL_WALL;

                terrainMap[get_map_index(x, y)] = changeInto;
            }
        }
    }
}
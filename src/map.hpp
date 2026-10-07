#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <utility>
#include <unordered_set>
#include <cstdint>
#include <source_location>

#include "common.hpp"
#include "game_config.hpp"

/// @brief Enum for things used in the map generation and rendering
///
/// Some things do not have textures associated with them.
enum Map {
    EMPTY,
    SPRITESHEET,
    SPIDER,
    PLAYER,

    // player idle
    player_girl_idle_down,
    player_girl_idle_down_left,
    player_girl_idle_down_right,
    player_girl_idle_right,
    player_girl_idle_left,
    player_girl_idle_up,
    player_girl_idle_up_left,
    player_girl_idle_up_right,
    // player walk
    player_girl_walk_down,
    player_girl_walk_down_left,
    player_girl_walk_down_right,
    player_girl_walk_right,
    player_girl_walk_left,
    player_girl_walk_up,
    player_girl_walk_up_left,
    player_girl_walk_up_right,
    // player run
    player_girl_run_down,
    player_girl_run_down_left,
    player_girl_run_down_right,
    player_girl_run_right,
    player_girl_run_left,
    player_girl_run_up,
    player_girl_run_up_left,
    player_girl_run_up_right,
    // player jump
    player_girl_jump_down,
    player_girl_jump_down_left,
    player_girl_jump_down_right,
    player_girl_jump_right,
    player_girl_jump_left,
    player_girl_jump_up,
    player_girl_jump_up_left,
    player_girl_jump_up_right,
    // player running-jump
    player_girl_running_jump_down,
    player_girl_running_jump_down_left,
    player_girl_running_jump_down_right,
    player_girl_running_jump_right,
    player_girl_running_jump_left,
    player_girl_running_jump_up,
    player_girl_running_jump_up_left,
    player_girl_running_jump_up_right,
    // player damage
    player_girl_damage_down,
    player_girl_damage_down_left,
    player_girl_damage_down_right,
    player_girl_damage_right,
    player_girl_damage_left,
    player_girl_damage_up,
    player_girl_damage_up_left,
    player_girl_damage_up_right,

    // spider run
    spider_run45_animation,
    spider_run135_animation,
    spider_run225_animation,
    spider_run315_animation,
    spider_run90_animation,
    spider_run180_animation,
    spider_run270_animation,
    spider_run0_animation,
    // spider walk
    spider_walk45_animation,
    spider_walk135_animation,
    spider_walk225_animation,
    spider_walk315_animation,
    spider_walk90_animation,
    spider_walk180_animation,
    spider_walk270_animation,
    spider_walk0_animation,
    // spider idle
    spider_idle45_animation,
    spider_idle135_animation,
    spider_idle225_animation,
    spider_idle315_animation,
    spider_idle90_animation,
    spider_idle180_animation,
    spider_idle270_animation,
    spider_idle0_animation,
    // spider raise
    spider_raise45_animation,
    spider_raise135_animation,
    spider_raise225_animation,
    spider_raise315_animation,
    spider_raise90_animation,
    spider_raise180_animation,
    spider_raise270_animation,
    spider_raise0_animation,
    // spider attack1
    spider_attack1_45_animation,
    spider_attack1_135_animation,
    spider_attack1_225_animation,
    spider_attack1_315_animation,
    spider_attack1_90_animation,
    spider_attack1_180_animation,
    spider_attack1_270_animation,
    spider_attack1_0_animation,
    // spider attack2
    spider_attack2_45_animation,
    spider_attack2_135_animation,
    spider_attack2_225_animation,
    spider_attack2_315_animation,
    spider_attack2_90_animation,
    spider_attack2_180_animation,
    spider_attack2_270_animation,
    spider_attack2_0_animation,

    SECTOR_1_PATHWAY,
    SECTOR_2_PATHWAY,
    SECTOR_3_PATHWAY,
    TREE_TRUNK,
    GROUND_CUBE,
    GROUND_CUBE_SPRITE,
    GRASS_COVER_SPRITE,
    GRASS_COVER,
    GRASS_COVER_SHORT,
    GRASS_COVER_TALL,
    TREE,
    MAZE_GROUND_CUBE,
    MAZE_GROUND_SPRITE,
    YELLOW_CUBE,
    ERROR_CUBE,
    BLUE_CUBE,
    RED_CUBE,
    WALL_CUBE,
    WALL_CUBE_GROUND,
    WALL_CUBE_SPRITE,
    INGROWN_WALL_CUBE,
    VINE_CUBE_HARD,
    VINE_CUBE_MEDIUM,
    VINE_CUBE_SOFT,
    SECTOR_1_WALL_VAL, // no texture
    SECTOR_2_WALL_VAL, // no texture
    SECTOR_3_WALL_VAL, // no texture
    VINE_OVERHANG_SN,
    VINE_OVERHANG_EW,
    VINE_COVER_N,
    VOID_CUBE,
    VOID_CUBE_NEIGHBOUR,
    INVISIBLE_CUBE,
    WALL_MARKINGS,
    MAZE_DECO,
    MAZE_NS_DOOR,
    MAZE_WE_DOOR,
    CAMPFIRE,
    WALL_SPRITESHEET,
    TUNNEL_PATHWAY,
    TUNNEL_WALL,
};

/// @brief Argument has to be a grid value of map
/// @param gridValue int 
/// @return Returns the const char* equivalent of that value from enum Map
inline constexpr const char* enumMapToString(int gridValue) {
    switch (gridValue) {
    case Map::SPRITESHEET: { return "Map::SPRITESHEET"; break; }
    case Map::SPIDER: { return "Map::SPIDER"; break; }
    case Map::PLAYER: { return "Map::PLAYER"; break; }
    case Map::player_girl_idle_down: { return "Map::player_girl_idle_down"; break; }
    case Map::player_girl_idle_down_left: { return "Map::player_girl_idle_down_left"; break; }
    case Map::player_girl_idle_down_right: { return "Map::player_girl_idle_down_right"; break; }
    case Map::player_girl_idle_right: { return "Map::player_girl_idle_right"; break; }
    case Map::player_girl_idle_left: { return "Map::player_girl_idle_left"; break; }
    case Map::player_girl_idle_up: { return "Map::player_girl_idle_up"; break; }
    case Map::player_girl_idle_up_left: { return "Map::player_girl_idle_up_left"; break; }
    case Map::player_girl_idle_up_right: { return "Map::player_girl_idle_up_right"; break; }
    case Map::player_girl_walk_down: { return "Map::player_girl_walk_down"; break; }
    case Map::player_girl_walk_down_left: { return "Map::player_girl_walk_down_left"; break; }
    case Map::player_girl_walk_down_right: { return "Map::player_girl_walk_down_right"; break; }
    case Map::player_girl_walk_right: { return "Map::player_girl_walk_right"; break; }
    case Map::player_girl_walk_left: { return "Map::player_girl_walk_left"; break; }
    case Map::player_girl_walk_up: { return "Map::player_girl_walk_up"; break; }
    case Map::player_girl_walk_up_left: { return "Map::player_girl_walk_up_left"; break; }
    case Map::player_girl_walk_up_right: { return "Map::player_girl_walk_up_right"; break; }
    case Map::player_girl_run_down: { return "Map::player_girl_run_down"; break; }
    case Map::player_girl_run_down_left: { return "Map::player_girl_run_down_left"; break; }
    case Map::player_girl_run_down_right: { return "Map::player_girl_run_down_right"; break; }
    case Map::player_girl_run_right: { return "Map::player_girl_run_right"; break; }
    case Map::player_girl_run_left: { return "Map::player_girl_run_left"; break; }
    case Map::player_girl_run_up: { return "Map::player_girl_run_up"; break; }
    case Map::player_girl_run_up_left: { return "Map::player_girl_run_up_left"; break; }
    case Map::player_girl_run_up_right: { return "Map::player_girl_run_up_right"; break; }
    case Map::player_girl_jump_down: { return "Map::player_girl_jump_down"; break; }
    case Map::player_girl_jump_down_left: { return "Map::player_girl_jump_down_left"; break; }
    case Map::player_girl_jump_down_right: { return "Map::player_girl_jump_down_right"; break; }
    case Map::player_girl_jump_right: { return "Map::player_girl_jump_right"; break; }
    case Map::player_girl_jump_left: { return "Map::player_girl_jump_left"; break; }
    case Map::player_girl_jump_up: { return "Map::player_girl_jump_up"; break; }
    case Map::player_girl_jump_up_left: { return "Map::player_girl_jump_up_left"; break; }
    case Map::player_girl_jump_up_right: { return "Map::player_girl_jump_up_right"; break; }
    case Map::player_girl_running_jump_down: { return "Map::player_girl_running_jump_down"; break; }
    case Map::player_girl_running_jump_down_left: { return "Map::player_girl_running_jump_down_left"; break; }
    case Map::player_girl_running_jump_down_right: { return "Map::player_girl_running_jump_down_right"; break; }
    case Map::player_girl_running_jump_right: { return "Map::player_girl_running_jump_right"; break; }
    case Map::player_girl_running_jump_left: { return "Map::player_girl_running_jump_left"; break; }
    case Map::player_girl_running_jump_up: { return "Map::player_girl_running_jump_up"; break; }
    case Map::player_girl_running_jump_up_left: { return "Map::player_girl_running_jump_up_left"; break; }
    case Map::player_girl_running_jump_up_right: { return "Map::player_girl_running_jump_up_right"; break; }
    case Map::player_girl_damage_down: { return "Map::player_girl_damage_down"; break; }
    case Map::player_girl_damage_down_left: { return "Map::player_girl_damage_down_left"; break; }
    case Map::player_girl_damage_down_right: { return "Map::player_girl_damage_down_right"; break; }
    case Map::player_girl_damage_right: { return "Map::player_girl_damage_right"; break; }
    case Map::player_girl_damage_left: { return "Map::player_girl_damage_left"; break; }
    case Map::player_girl_damage_up: { return "Map::player_girl_damage_up"; break; }
    case Map::player_girl_damage_up_left: { return "Map::player_girl_damage_up_left"; break; }
    case Map::player_girl_damage_up_right: { return "Map::player_girl_damage_up_right"; break; }
    case Map::spider_run45_animation: { return "Map::spider_run45_animation"; break; }
    case Map::spider_run135_animation: { return "Map::spider_run135_animation"; break; }
    case Map::spider_run225_animation: { return "Map::spider_run225_animation"; break; }
    case Map::spider_run315_animation: { return "Map::spider_run315_animation"; break; }
    case Map::spider_run90_animation: { return "Map::spider_run90_animation"; break; }
    case Map::spider_run180_animation: { return "Map::spider_run180_animation"; break; }
    case Map::spider_run270_animation: { return "Map::spider_run270_animation"; break; }
    case Map::spider_run0_animation: { return "Map::spider_run0_animation"; break; }
    case Map::spider_walk45_animation: { return "Map::spider_walk45_animation"; break; }
    case Map::spider_walk135_animation: { return "Map::spider_walk135_animation"; break; }
    case Map::spider_walk225_animation: { return "Map::spider_walk225_animation"; break; }
    case Map::spider_walk315_animation: { return "Map::spider_walk315_animation"; break; }
    case Map::spider_walk90_animation: { return "Map::spider_walk90_animation"; break; }
    case Map::spider_walk180_animation: { return "Map::spider_walk180_animation"; break; }
    case Map::spider_walk270_animation: { return "Map::spider_walk270_animation"; break; }
    case Map::spider_walk0_animation: { return "Map::spider_walk0_animation"; break; }
    case Map::spider_idle45_animation: { return "Map::spider_idle45_animation"; break; }
    case Map::spider_idle135_animation: { return "Map::spider_idle135_animation"; break; }
    case Map::spider_idle225_animation: { return "Map::spider_idle225_animation"; break; }
    case Map::spider_idle315_animation: { return "Map::spider_idle315_animation"; break; }
    case Map::spider_idle90_animation: { return "Map::spider_idle90_animation"; break; }
    case Map::spider_idle180_animation: { return "Map::spider_idle180_animation"; break; }
    case Map::spider_idle270_animation: { return "Map::spider_idle270_animation"; break; }
    case Map::spider_idle0_animation: { return "Map::spider_idle0_animation"; break; }
    case Map::spider_raise45_animation: { return "Map::spider_raise45_animation"; break; }
    case Map::spider_raise135_animation: { return "Map::spider_raise135_animation"; break; }
    case Map::spider_raise225_animation: { return "Map::spider_raise225_animation"; break; }
    case Map::spider_raise315_animation: { return "Map::spider_raise315_animation"; break; }
    case Map::spider_raise90_animation: { return "Map::spider_raise90_animation"; break; }
    case Map::spider_raise180_animation: { return "Map::spider_raise180_animation"; break; }
    case Map::spider_raise270_animation: { return "Map::spider_raise270_animation"; break; }
    case Map::spider_raise0_animation: { return "Map::spider_raise0_animation"; break; }
    case Map::SECTOR_1_PATHWAY: { return "Map::SECTOR_1_PATHWAY"; break; }
    case Map::SECTOR_2_PATHWAY: { return "Map::SECTOR_2_PATHWAY"; break; }
    case Map::SECTOR_3_PATHWAY: { return "Map::SECTOR_3_PATHWAY"; break; }
    case Map::TREE_TRUNK: { return "Map::TREE_TRUNK"; break; }
    case Map::GROUND_CUBE: { return "Map::GROUND_CUBE"; break; }
    case Map::GROUND_CUBE_SPRITE: { return "Map::GROUND_CUBE_SPRITE"; break; }
    case Map::GRASS_COVER_SPRITE: { return "Map::GRASS_COVER_SPRITE"; break; }
    case Map::GRASS_COVER: { return "Map::GRASS_COVER"; break; }
    case Map::GRASS_COVER_SHORT: { return "Map::GRASS_COVER_SHORT"; break; }
    case Map::GRASS_COVER_TALL: { return "Map::GRASS_COVER_TALL"; break; }
    case Map::TREE: { return "Map::TREE"; break; }
    case Map::MAZE_GROUND_CUBE: { return "Map::MAZE_GROUND_CUBE"; break; }
    case Map::MAZE_GROUND_SPRITE: { return "Map::MAZE_GROUND_SPRITE"; break; }
    case Map::YELLOW_CUBE: { return "Map::YELLOW_CUBE"; break; }
    case Map::ERROR_CUBE: { return "Map::ERROR_CUBE"; break; }
    case Map::BLUE_CUBE: { return "Map::BLUE_CUBE"; break; }
    case Map::WALL_CUBE: { return "Map::WALL_CUBE"; break; }
    case Map::WALL_CUBE_GROUND: { return "Map::WALL_CUBE_GROUND"; break; }
    case Map::WALL_CUBE_SPRITE: { return "Map::WALL_CUBE_SPRITE"; break; }
    case Map::INGROWN_WALL_CUBE: { return "Map::INGROWN_WALL_CUBE"; break; }
    case Map::VINE_CUBE_HARD: { return "Map::VINE_CUBE_HARD"; break; }
    case Map::VINE_CUBE_MEDIUM: { return "Map::VINE_CUBE_MEDIUM"; break; }
    case Map::VINE_CUBE_SOFT: { return "Map::VINE_CUBE_SOFT"; break; }
    case Map::SECTOR_1_WALL_VAL: { return "Map::SECTOR_1_WALL_VAL"; break; }
    case Map::SECTOR_2_WALL_VAL: { return "Map::SECTOR_2_WALL_VAL"; break; }
    case Map::SECTOR_3_WALL_VAL: { return "Map::SECTOR_3_WALL_VAL"; break; }
    case Map::VINE_OVERHANG_SN: { return "Map::VINE_OVERHANG_SN"; break; }
    case Map::VINE_OVERHANG_EW: { return "Map::VINE_OVERHANG_EW"; break; }
    case Map::VINE_COVER_N: { return "Map::VINE_COVER_N"; break; }
    case Map::VOID_CUBE: { return "Map::VOID_CUBE"; break; }
    case Map::VOID_CUBE_NEIGHBOUR: { return "Map::VOID_CUBE_NEIGHBOUR"; break; }
    case Map::INVISIBLE_CUBE: { return "Map::INVISIBLE_CUBE"; break; }
    case Map::WALL_MARKINGS: { return "Map::WALL_MARKINGS"; break; }
    case Map::MAZE_DECO: { return "Map::MAZE_DECO"; break; }
    case Map::MAZE_NS_DOOR: { return "Map::MAZE_NS_DOOR"; break; }
    case Map::MAZE_WE_DOOR: { return "Map::MAZE_WE_DOOR"; break; }
    case Map::CAMPFIRE: { return "Map::CAMPFIRE"; break; }
    case Map::WALL_SPRITESHEET: { return "Map::WALL_SPRITESHEET"; break; }
    case Map::TUNNEL_PATHWAY: { return "Map::TUNNEL_PATHWAY"; break; }
    case Map::TUNNEL_WALL: { return "Map::TUNNEL_WALL"; break; }
    case Map::EMPTY: { return "Map::EMPTY"; break; }
    case Map::RED_CUBE: { return "Map::RED_CUBE"; break; }
    default: { return "Unconfigured Map enum"; break; }
    }
}


static std::unordered_set<int> wallValues = {
    Map::WALL_CUBE, Map::INGROWN_WALL_CUBE,
    Map::SECTOR_1_WALL_VAL, Map::SECTOR_2_WALL_VAL, Map::SECTOR_3_WALL_VAL,
    Map::TUNNEL_WALL
};

static const std::unordered_set<int> groundValues = {
    Map::GROUND_CUBE, Map::TREE, Map::GRASS_COVER, Map::TREE_TRUNK
};

/* map */
extern std::vector<int> terrainMap;
std::set<std::pair<int, int>>& getS3PatchLocations();
extern std::unordered_set<uint32_t> sector3Cutouts;

inline int get_map_index(int x, int y, const std::source_location loc = std::source_location::current()) {
    if (x < 0 || x >= MAP_SIDE_LENGTH || y < 0 || y >= MAP_SIDE_LENGTH) {
        std::cout << "CRASH PREVENTION: Out of bounds at "
            << loc.file_name() << ":"
            << loc.line() << " in "
            << loc.function_name()
            << " — X: " << x << ", Y: " << y << std::endl;
        return 0;
    }
    return (y * MAP_SIDE_LENGTH) + x;
};

// how often should some decorations spawn
const int mazeDecoMod = 4;
const int groundDecoMod = 4;
const int vinesDecoMod = 4;
const int groundMod = 12;
// how often a sector3 wall will have a lower-wall cutout (1 in N)
const int sector3CutoutMod = 3;

namespace MapGenerator {
    void init();

    void generate_ground();
    void generate_mazes();
    void generate_s1_clearspots();
    void generate_glade(std::vector<int>& textureMap);
    void generate_decorations();
    void generate_voids();
    void generate_cliffs();
    void generate_tunnels();
    void generate_test(std::vector<int>& textureMap);
    void generate_s3_patches(int patchWidth = 5);
}
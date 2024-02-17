#pragma once
#include <filesystem>
#include <vector>

#define WORLD_NUM 7

struct Room {
    std::filesystem::path path;
};

struct World {
    std::vector<Room> levels[WORLD_NUM];
    int levelIndex = 0;
};

namespace Config {

World initWorld();

}

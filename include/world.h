#pragma once
#include <DirectXMath.h>

#include <filesystem>
#include <vector>

#define WORLD_NUM 7

struct Geometry {
    std::vector<DirectX::XMFLOAT3> pos;
    std::vector<DirectX::XMFLOAT3> uvs;
    std::vector<DirectX::XMFLOAT3> norm;
    std::vector<unsigned int> indices;
};

struct Room {
    std::filesystem::path path;
};

struct World {
    std::vector<Room> levels[WORLD_NUM];
    int levelIndex = 0;
};

namespace Config {

World initWorld();
void loadRoom(World &world, int roomIndex);

}

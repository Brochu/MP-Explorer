#pragma once
#include <DirectXMath.h>

#include <filesystem>
#include <vector>

#define WORLD_NUM 7

struct Geometry {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 uvs;
    DirectX::XMFLOAT3 norm;
};

struct Room {
    std::filesystem::path path;

    int voffset;
    unsigned int vcount;
    int ioffset;
    unsigned int icount;
};

struct World {
    std::vector<Room> levels[WORLD_NUM];
    int levelIndex = 1;
    std::vector<Geometry> vertices;
    std::vector<unsigned int> indices;
};

namespace Config {

World initWorld();
void loadRoom(World &world, int roomIndex);

}

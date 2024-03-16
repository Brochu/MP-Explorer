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

struct Mesh {
    std::vector<Geometry> vertices;
    std::vector<unsigned int> indices;
};

struct Room {
    std::filesystem::path path;
    std::vector<Mesh> meshes;
};

struct World {
    std::vector<Room> levels[WORLD_NUM];
    int levelIndex = 1;
};

namespace Config {

World initWorld();
void loadRoom(World &world, int roomIndex);

}

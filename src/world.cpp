#include "world.h"

#include "assimp/Importer.hpp"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <stdio.h>

namespace Config {
using namespace std::filesystem;

typedef unsigned int uint;

const char *levelFolders[WORLD_NUM] {
    "Metroid1\\!1IntroWorld0310a_158EFE17",
    "Metroid2\\!2RuinsWorld0310_83F6FF6F",
    "Metroid3\\!3IceWorld0310_A8BE6291",
    "Metroid4\\!4OverWorld0310_39F2DE28",
    "Metroid5\\!5MinesWorld0310_B1AC4D65",
    "Metroid6\\!6LavaWorld0310_3EF8237C",
    "Metroid7\\!7CraterWorld0310_C13B09D1",
};

World initWorld() {
    World w;

    for (int i = 0; i < WORLD_NUM; i++) {
        std::string path(PATH);
        path.resize(255); //TODO: Maybe change this max value later
        sprintf_s(path.data(), 255, "%s\\%s",
            PATH,
            levelFolders[i]
        );

        for (directory_entry entry : directory_iterator(path.c_str())) {
            if (entry.is_directory()) {
                w.levels[i].push_back({ entry.path().filename() });
            }
        }
    }

    return w;
}

void loadRoom(World &world, int roomIndex) {
    //TODO: Load room data
    // How do we read YAML files
    // Open and read !area file
    // We need to store all objs (verts + idx)
    // Look through default folder only to start
    Room r = world.levels[world.levelIndex][roomIndex];

    std::string path;
    path.resize(255); //TODO: Maybe change this max value later
    sprintf_s(path.data(), 255, "%s\\%s\\%ls", PATH, levelFolders[world.levelIndex], r.path.c_str());

    directory_entry model;
    for (directory_entry entry : directory_iterator(path.c_str())) {
        std::wstring name = entry.path().filename().c_str();
        //TODO: Maybe test if loading from format != .blend would give better speed?
        if (name.find(L"area") != std::wstring::npos) {
            model = entry;
            break;
        }
    }

    sprintf_s(path.data(), 255, "%ls", model.path().c_str());
    Assimp::Importer imp;
    const aiScene *scene = imp.ReadFile(
        path.c_str(),
        aiProcessPreset_TargetRealtime_Quality | aiProcess_ConvertToLeftHanded
    );
    //TODO: Need to check if we can read shaders from .blend files

    std::vector<aiNode*> stack;
    for(uint i = 0; i < scene->mRootNode->mNumChildren; i++) {
        if (strcmp(scene->mRootNode->mChildren[i]->mName.C_Str(), "Render") == 0) {
            stack.push_back(scene->mRootNode->mChildren[i]);
            break;
        }
    }

    while(!stack.empty()) {
        aiNode *n = stack.back();
        stack.pop_back();
        for (uint i = 0; i < n->mNumMeshes; i++) {
            //TODO: Extract all vertices/indices
            aiMesh *m = scene->mMeshes[n->mMeshes[i]];
            size_t midx = r.meshes.size();
            r.meshes.emplace_back();

            for (uint j = 0; j < m->mNumVertices; j++) {
                size_t vidx = r.meshes[midx].vertices.size();
                aiVector3D vpos = m->mVertices[j];
                aiVector3D norm = m->mNormals[j];
                aiVector3D uvs = m->mTextureCoords[0][j];

                r.meshes[midx].vertices.emplace_back();
                r.meshes[midx].vertices[vidx].pos = { vpos.x, vpos.y, vpos.z };
                r.meshes[midx].vertices[vidx].norm = { norm.x, norm.y, norm.z };
                r.meshes[midx].vertices[vidx].uvs = { uvs.x, uvs.y };
            }

            for (uint j = 0; j < m->mNumFaces; j++) {
                aiFace face = m->mFaces[j];

                for (uint k = 0; k < face.mNumIndices; k++) {
                    r.meshes[midx].indices.push_back(face.mIndices[k]);
                }
            }
        }

        for(uint i = 0; i < n->mNumChildren; i++) {
            stack.insert(stack.begin(), n->mChildren[i]);
        }
    }
}

}

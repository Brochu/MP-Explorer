#include "world.h"
#include <stdio.h>

namespace Config {
using namespace std::filesystem;

const char *levelFolders[WORLD_NUM] {
    "\\Metroid1\\!1IntroWorld0310a_158EFE17",
    "\\Metroid2\\!2RuinsWorld0310_83F6FF6F",
    "\\Metroid3\\!3IceWorld0310_A8BE6291",
    "\\Metroid4\\!4OverWorld0310_39F2DE28",
    "\\Metroid5\\!5MinesWorld0310_B1AC4D65",
    "\\Metroid6\\!6LavaWorld0310_3EF8237C",
    "\\Metroid7\\!7CraterWorld0310_C13B09D1",
};

World initWorld() {
    World w;

    for (int i = 0; i < WORLD_NUM; i++) {
        std::string path(PATH);
        path.append(levelFolders[i]);

        printf("[WORLD] Loading rooms from %s\n", path.c_str());
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
    std::string path;
    path.resize(255); //TODO: Maybe change this max value later
    sprintf_s(path.data(), 255, "%s\\%s\\%ls",
        PATH,
        levelFolders[world.levelIndex],
        world.levels[world.levelIndex][roomIndex].path.c_str()
    );
    printf("[WORLD] Room='%s'\n", path.c_str());
    for (directory_entry entry : directory_iterator(path.c_str())) {
        printf(" - '%ls'\n", entry.path().filename().c_str());
    }
}

}

#include "app.h"

#include <cstdio>
#include <filesystem>

int main(int argc, char **argv) {
    //TODO: Parse command line args

    printf("[APP] Data path is setup %s\n", PATH);
    for (const auto &entry : std::filesystem::directory_iterator(PATH)) {
        printf(" - '%ls'\n", entry.path().c_str());
    }

    int ret = App::run();
    printf("[MAIN] Done running, return code: %i\n", ret);

    return ret;
}

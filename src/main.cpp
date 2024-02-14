#include "app.h"
#include "cstdio"

int main(int argc, char **argv) {
    //TODO: Parse command line args

    int ret = App::run();
    printf("[MAIN] Done running, return code: %i\n", ret);

    return ret;
}

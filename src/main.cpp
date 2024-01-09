#include "app.h"

int main(int argc, char **argv) {
    App::setup();

    while(App::running()) {
        App::step();
    }

    App::teardown();
    return 0;
}

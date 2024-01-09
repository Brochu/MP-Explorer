#include "app.h"
#include "renderer.h"

int main(int argc, char **argv) {

    App::setup();
    Render::setup();

    while(App::running) {
        App::step();
    }

    App::teardown();
    return 0;
}

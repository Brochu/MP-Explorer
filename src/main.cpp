#include "app.h"

int main(int argc, char **argv) {
    App::Setup();

    while(App::running) {
        App::Step();
    }

    App::Teardown();
    return 0;
}

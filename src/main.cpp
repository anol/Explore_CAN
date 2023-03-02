
#include "Explore_CAN.h"

Explore_CAN explorer{};

extern "C" int main(void) {
    explorer.run();
}
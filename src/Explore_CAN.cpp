//
// Created by aeols on 02.03.2023.
//

#include "Explore_CAN.h"


[[noreturn]] void Explore_CAN::run() {
    the_CH32.initialize();
    printf("\r\nExplore CAN\r\n");
    the_CH32.print_info();
    the_CAN.initialize();
    the_CAN.print_info();
    the_LAN.initialize();
    the_LAN.print_info();
    while (true) {
        the_LAN.task();
        the_CAN.task();
    }
}

//
// Created by aeols on 02.03.2023.
//

#ifndef EXPLORE_CAN_EXPLORE_CAN_H
#define EXPLORE_CAN_EXPLORE_CAN_H


#include "Handle_CAN.h"
#include "Handle_LAN.h"
#include "Handle_MCU.h"

class Explore_CAN {
    Handle_MCU the_CH32{};
    Handle_CAN the_CAN{the_CH32};
    Handle_LAN the_LAN{the_CH32};

public:
    [[noreturn]] void run();
};


#endif //EXPLORE_CAN_EXPLORE_CAN_H

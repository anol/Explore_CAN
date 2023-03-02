//
// Created by aeols on 02.03.2023.
//

#ifndef EXPLORE_CAN_EXPLORE_CAN_H
#define EXPLORE_CAN_EXPLORE_CAN_H


#include "CAN_handler.h"
#include "LAN_handler.h"
#include "CH32_platform.h"

class Explore_CAN {
    CH32_platform the_CH32{};
    CAN_handler the_CAN{the_CH32};
    LAN_handler the_LAN{the_CH32};

public:
    void run();
};


#endif //EXPLORE_CAN_EXPLORE_CAN_H

//
// Created by aeols on 02.03.2023.
//

#ifndef EXPLORE_CAN_LAN_HANDLER_H
#define EXPLORE_CAN_LAN_HANDLER_H


#include "CH32_platform.h"

class LAN_handler {
    CH32_platform &use_CH32;
public:
    explicit LAN_handler(CH32_platform &CH32) : use_CH32(CH32) {}

    void initialize();
};


#endif //EXPLORE_CAN_LAN_HANDLER_H

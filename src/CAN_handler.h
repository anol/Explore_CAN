//
// Created by aeols on 02.03.2023.
//

#ifndef EXPLORE_CAN_CAN_HANDLER_H
#define EXPLORE_CAN_CAN_HANDLER_H


#include "CH32_platform.h"

class CAN_handler {
    CH32_platform &use_CH32;
    uint32_t the_next_time{};

public:
    explicit CAN_handler(CH32_platform &CH32) : use_CH32(CH32) {}

    void initialize();

    void task();

private:
    void setup_CAN(u8 tsjw, u8 tbs2, u8 tbs1, u16 brp, u8 mode);

    u8 transmit(u8 *msg, u8 len, u32 id, bool extended);

    u8 receive(u8 *buf);
};


#endif //EXPLORE_CAN_CAN_HANDLER_H

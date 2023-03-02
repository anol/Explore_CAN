//
// Created by aeols on 02.03.2023.
//

#ifndef EXPLORE_CAN_HANDLE_CAN_H
#define EXPLORE_CAN_HANDLE_CAN_H


#include "Handle_MCU.h"

class Handle_CAN {
    Handle_MCU &use_CH32;
    uint32_t the_next_time{};
    u8 tsjw;
    u8 tbs2;
    u8 tbs1;
    u16 brp;
public:
    explicit Handle_CAN(Handle_MCU &CH32);

    void initialize();

    void print_info() const;

    void task();

private:
    void setup_CAN(u8 mode);

    u8 transmit(u8 *msg, u8 len, u32 id, bool extended);

    u8 receive(u8 *buf);
};


#endif //EXPLORE_CAN_HANDLE_CAN_H

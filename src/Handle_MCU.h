//
// Created by aeols on 02.03.2023.
//

#ifndef EXPLORE_CAN_HANDLE_MCU_H
#define EXPLORE_CAN_HANDLE_MCU_H

#include <cstdint>

extern "C" {
#include "string.h"
#include "debug.h"
#include "wchnet.h"
#include "eth_driver.h"
}

class Handle_MCU {
    uint32_t the_chip_id{};
    uint32_t the_clock{};

public:
    static uint32_t the_timex;

public:
    void initialize();

    void print_info() const;

    uint32_t get_timex() const { return the_timex; }

private:
    static void setup_TIM2();

    static void setup_TIM3();
};


#endif //EXPLORE_CAN_HANDLE_MCU_H

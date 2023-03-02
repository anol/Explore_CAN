//
// Created by aeols on 02.03.2023.
//

#ifndef EXPLORE_CAN_HANDLE_LAN_H
#define EXPLORE_CAN_HANDLE_LAN_H


#include "Handle_MCU.h"

class Handle_LAN {
    Handle_MCU &use_platform;
    u8 MACAddr[6]{};
    u8 IPAddr[4]{};
    u8 GWIPAddr[4]{};
    u8 IPMask[4]{};

public:
    explicit Handle_LAN(Handle_MCU &CH32) : use_platform(CH32) {}

    void initialize();

    void print_info() const;

    void task();

private:
    void create_TCP_listen();

    void loopback(u8 id);

    void handle_interrupt();

    void handle_socket_interrupt(u8 socketid, u8 intstat);

    u8 DHCP_callback(u8 status, void *arg);
};


#endif //EXPLORE_CAN_HANDLE_LAN_H

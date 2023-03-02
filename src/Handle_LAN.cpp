//
// Created by aeols on 02.03.2023.
//

#include "Handle_LAN.h"

extern "C" {
#include "string.h"
#include "debug.h"
#include "wchnet.h"
#include "eth_driver.h"
}


#define USE_STATIC_IP                   1
#define KEEPALIVE_ENABLE                1                //Enable keep alive function

u8 MACAddr[6];                                          //MAC address
#ifdef USE_STATIC_IP
u8 IPAddr[4] = {192, 168, 1, 77};                     //IP address
u8 GWIPAddr[4] = {192, 168, 1, 1};                    //Gateway IP address
u8 IPMask[4] = {255, 255, 255, 0};                    //subnet mask
#else
u8 IPAddr[4] = {0, 0, 0, 0};                    //IP address
u8 GWIPAddr[4] = {0, 0, 0, 0};                    //Gateway IP address
u8 IPMask[4] = {0, 0, 0, 0};                    //subnet mask
#endif

u16 srcport = 1000;                                     //source port

u8 SocketIdForListen;                                   //Socket for Listening
u8 socket[WCHNET_MAX_SOCKET_NUM];                       //Save the currently connected socket
u8 SocketRecvBuf[WCHNET_MAX_SOCKET_NUM][RECE_BUF_LEN];  //socket receive buffer
u8 MyBuf[RECE_BUF_LEN];


void Handle_LAN::initialize() {
    WCHNET_GetMacAddr(MACAddr);
    if (WCHNET_LIB_VER != WCHNET_GetVer()) {
        printf("version error.\r\n");
    }
#ifndef USE_STATIC_IP
    WCHNET_DHCPSetHostname("Explore_CAN");
#endif
    auto i = ETH_LibInit(IPAddr, GWIPAddr, IPMask, MACAddr);           //Ethernet library initialize
    if (i == WCHNET_ERR_SUCCESS)
        printf("WCHNET_LibInit Success\r\n");
#if KEEPALIVE_ENABLE                                               //Configure keep alive parameters
    {
        struct _KEEP_CFG cfg;
        cfg.KLIdle = 20000;
        cfg.KLIntvl = 15000;
        cfg.KLCount = 9;
        WCHNET_ConfigKeepLive(&cfg);
    }
#endif
#ifdef USE_STATIC_IP
    memset(socket, 0xff, WCHNET_MAX_SOCKET_NUM);
    create_TCP_listen();                               //Create TCP Socket for Listening
#else
    WCHNET_DHCPStart(WCHNET_DHCPCallBack);                                //Start DHCP
#endif
}

void Handle_LAN::print_info() const {
    printf("net version:%x\r\n", WCHNET_GetVer());
    printf("mac addr:");
    for (unsigned char i: MACAddr) printf("%x ", i);
    printf("\r\n");
}

void Handle_LAN::task() {
    WCHNET_MainTask();
    if (WCHNET_QueryGlobalInt()) {
        handle_interrupt();
    }
}

void mStopIfError(u8 iError) {
    if (iError == WCHNET_ERR_SUCCESS)
        return;
    printf("Error: %02X\r\n", (u16) iError);
}

void Handle_LAN::create_TCP_listen(void) {
    u8 i;
    SOCK_INF TmpSocketInf;
    memset((void *) &TmpSocketInf, 0, sizeof(SOCK_INF));
    TmpSocketInf.SourPort = srcport;
    TmpSocketInf.ProtoType = PROTO_TYPE_TCP;
    i = WCHNET_SocketCreat(&SocketIdForListen, &TmpSocketInf);
    printf("SocketIdForListen %d\r\n", SocketIdForListen);
    mStopIfError(i);
    i = WCHNET_SocketListen(SocketIdForListen);                   //listen for connections
    mStopIfError(i);
}

void Handle_LAN::loopback(u8 id) {
#if 1
    u8 i;
    u32 len;
    //Receive buffer end address
    u32 endAddr = SocketInf[id].RecvStartPoint + SocketInf[id].RecvBufLen;
    if ((SocketInf[id].RecvReadPoint + SocketInf[id].RecvRemLen) > endAddr) {
        //Calculate the length of the received data
        len = endAddr - SocketInf[id].RecvReadPoint;
    } else {
        len = SocketInf[id].RecvRemLen;
    }
    i = WCHNET_SocketSend(id, (u8 *) SocketInf[id].RecvReadPoint, &len);        //send data
    if (i == WCHNET_ERR_SUCCESS) {
        WCHNET_SocketRecv(id, NULL, &len);                                      //Clear sent data
    }
#else
    u32 len, totallen;
    u8 *p = MyBuf;

    len = WCHNET_SocketRecvLen(id, NULL);                                //query length
    printf("Receive Len = %02x\n", len);
    totallen = len;
    WCHNET_SocketRecv(id, MyBuf, &len);                                  //Read the data of the receive buffer into MyBuf
    while(1){
        len = totallen;
        WCHNET_SocketSend(id, p, &len);                                  //Send the data
        totallen -= len;                                                 //Subtract the sent length from the total length
        p += len;                                                        //offset buffer pointer
        if(totallen)continue;                                            //If the data is not sent, continue to send
        break;                                                           //After sending, exit
    }
#endif
}

void Handle_LAN::handle_socket_interrupt(u8 socketid, u8 intstat) {
    u8 i;

    if (intstat & SINT_STAT_RECV)                                 //receive data
    {
        loopback(socketid);                            //Data loopback
    }
    if (intstat & SINT_STAT_CONNECT)                              //connect successfully
    {
#if KEEPALIVE_ENABLE
        WCHNET_SocketSetKeepLive(socketid, ENABLE);
#endif
        WCHNET_ModifyRecvBuf(socketid, (u32) SocketRecvBuf[socketid],
                             RECE_BUF_LEN);
        for (i = 0; i < WCHNET_MAX_SOCKET_NUM; i++) {
            if (socket[i] == 0xff) {                              //save connected socket id
                socket[i] = socketid;
                break;
            }
        }
        printf("TCP Connect Success\r\n");
        printf("socket id: %d\r\n", socket[i]);
    }
    if (intstat & SINT_STAT_DISCONNECT)                           //disconnect
    {
        for (i = 0; i < WCHNET_MAX_SOCKET_NUM; i++) {             //delete disconnected socket id
            if (socket[i] == socketid) {
                socket[i] = 0xff;
                break;
            }
        }
        printf("TCP Disconnect\r\n");
    }
    if (intstat & SINT_STAT_TIM_OUT)                              //timeout disconnect
    {
        for (i = 0; i < WCHNET_MAX_SOCKET_NUM; i++) {             //delete disconnected socket id
            if (socket[i] == socketid) {
                socket[i] = 0xff;
                break;
            }
        }
        printf("TCP Timeout\r\n");
    }
}

void Handle_LAN::handle_interrupt( ) {
    u8 intstat;
    u16 i;
    u8 socketint;

    intstat = WCHNET_GetGlobalInt();                              //get global interrupt flag
    if (intstat & GINT_STAT_UNREACH)                              //Unreachable interrupt
    {
        printf("GINT_STAT_UNREACH\r\n");
    }
    if (intstat & GINT_STAT_IP_CONFLI)                            //IP conflict
    {
        printf("GINT_STAT_IP_CONFLI\r\n");
    }
    if (intstat & GINT_STAT_PHY_CHANGE)                           //PHY status change
    {
        i = WCHNET_GetPHYStatus();
        if (i & PHY_Linked_Status)
            printf("PHY Link Success\r\n");
    }
    if (intstat & GINT_STAT_SOCKET) {                             //socket related interrupt
        for (i = 0; i < WCHNET_MAX_SOCKET_NUM; i++) {
            socketint = WCHNET_GetSocketInt(i);
            if (socketint)
                handle_socket_interrupt(i, socketint);
        }
    }
}

u8 Handle_LAN::DHCP_callback(u8 status, void *arg) {
    u8 *p;
    u8 tmp[4] = {0, 0, 0, 0};

    if (!status) {
        p = static_cast<u8 *>(arg);
        printf("DHCP Success\r\n");
        /*If the obtained IP is the same as the last IP, exit this function.*/
        if (!memcmp(IPAddr, p, sizeof(IPAddr)))
            return READY;
        /*Determine whether it is the first successful IP acquisition*/
        if (memcmp(IPAddr, tmp, sizeof(IPAddr))) {
            /*The obtained IP is different from the last value,
             * then disconnect the last connection.*/
            WCHNET_SocketClose(SocketIdForListen, TCP_CLOSE_NORMAL);
        }
        memcpy(IPAddr, p, 4);
        memcpy(GWIPAddr, &p[4], 4);
        memcpy(IPMask, &p[8], 4);
        printf("IPAddr = %d.%d.%d.%d \r\n", (u16) IPAddr[0], (u16) IPAddr[1],
               (u16) IPAddr[2], (u16) IPAddr[3]);
        printf("GWIPAddr = %d.%d.%d.%d \r\n", (u16) GWIPAddr[0], (u16) GWIPAddr[1],
               (u16) GWIPAddr[2], (u16) GWIPAddr[3]);
        printf("IPAddr = %d.%d.%d.%d \r\n", (u16) IPMask[0], (u16) IPMask[1],
               (u16) IPMask[2], (u16) IPMask[3]);
        printf("DNS1: %d.%d.%d.%d \r\n", p[12], p[13], p[14], p[15]);
        printf("DNS2: %d.%d.%d.%d \r\n", p[16], p[17], p[18], p[19]);

        u8 i = ETH_LibInit(IPAddr, GWIPAddr, IPMask, MACAddr);           //Ethernet library initialize
        mStopIfError(i);
        if (i == WCHNET_ERR_SUCCESS)
            printf("ETH_LibInit Success\r\n");

        memset(socket, 0xff, WCHNET_MAX_SOCKET_NUM);
        create_TCP_listen();                               //Create TCP Socket for Listening

        return READY;
    } else {
        printf("DHCP Fail %02x \r\n", status);
        /*Determine whether it is the first successful IP acquisition*/
        if (memcmp(IPAddr, tmp, sizeof(IPAddr))) {
            /*The obtained IP is different from the last value*/
            WCHNET_SocketClose(SocketIdForListen, TCP_CLOSE_NORMAL);
        }
        return NoREADY;
    }
}

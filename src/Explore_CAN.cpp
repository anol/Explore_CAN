//
// Created by aeols on 02.03.2023.
//

#include "Explore_CAN.h"


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

/*********************************************************************
 * @fn      mStopIfError
 *
 * @brief   check if error.
 *
 * @param   iError - error constants.
 *
 * @return  none
 */
void mStopIfError(u8 iError) {
    if (iError == WCHNET_ERR_SUCCESS)
        return;
    printf("Error: %02X\r\n", (u16) iError);
}


/*********************************************************************
 * @fn      WCHNET_CreateTcpSocketListen
 *
 * @brief   Create TCP Socket for Listening
 *
 * @return  none
 */
void WCHNET_CreateTcpSocketListen(void) {
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

/*********************************************************************
 * @fn      WCHNET_DataLoopback
 *
 * @brief   Data loopback function.
 *
 * @param   id - socket id.
 *
 * @return  none
 */
void WCHNET_DataLoopback(u8 id) {
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

/*********************************************************************
 * @fn      WCHNET_HandleSockInt
 *
 * @brief   Socket Interrupt Handle
 *
 * @param   socketid - socket id.
 *          intstat - interrupt status
 *
 * @return  none
 */
void WCHNET_HandleSockInt(u8 socketid, u8 intstat) {
    u8 i;

    if (intstat & SINT_STAT_RECV)                                 //receive data
    {
        WCHNET_DataLoopback(socketid);                            //Data loopback
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

/*********************************************************************
 * @fn      WCHNET_HandleGlobalInt
 *
 * @brief   Global Interrupt Handle
 *
 * @return  none
 */
void WCHNET_HandleGlobalInt(void) {
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
                WCHNET_HandleSockInt(i, socketint);
        }
    }
}

/*********************************************************************
 * @fn      WCHNET_DHCPCallBack
 *
 * @brief   DHCPCallBack
 *
 * @param   status - status returned by DHCP
 *          arg - Data returned by DHCP
 *
 * @return  DHCP status
 */
u8 WCHNET_DHCPCallBack(u8 status, void *arg) {
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
        WCHNET_CreateTcpSocketListen();                               //Create TCP Socket for Listening

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

void Explore_CAN::run() {
    u8 i;
    the_CH32.initialize();
    printf("\r\nExplore CAN\r\n");
    the_CH32.print_info();
    the_CAN.initialize();
    the_LAN.initialize();

    printf("net version:%x\r\n", WCHNET_GetVer());
    if (WCHNET_LIB_VER != WCHNET_GetVer()) {
        printf("version error.\r\n");
    }
    WCHNET_GetMacAddr(MACAddr);                                   //get the chip MAC address
    printf("mac addr:");
    for (i = 0; i < 6; i++)
        printf("%x ", MACAddr[i]);
    printf("\r\n");
#ifndef USE_STATIC_IP
    WCHNET_DHCPSetHostname("Explore_CAN");                                   //Configure DHCP host name
#endif
    i = ETH_LibInit(IPAddr, GWIPAddr, IPMask, MACAddr);           //Ethernet library initialize
    mStopIfError(i);
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
    WCHNET_CreateTcpSocketListen();                               //Create TCP Socket for Listening
#else
    WCHNET_DHCPStart(WCHNET_DHCPCallBack);                                //Start DHCP
#endif

    while (1) {
        /*Ethernet library main task function,
         * which needs to be called cyclically*/
        WCHNET_MainTask();
        /*Query the Ethernet global interrupt,
         * if there is an interrupt, call the global interrupt handler*/
        if (WCHNET_QueryGlobalInt()) {
            WCHNET_HandleGlobalInt();
        }

        the_CAN.task();
    }
}
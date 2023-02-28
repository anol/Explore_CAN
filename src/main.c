/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/05/31
 * Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
/*
 *@Note
TCP Server example, demonstrating that TCP Server
receives data and sends back after connecting to a client.
For details on the selection of engineering chips,
please refer to the "CH32V30x Evaluation Board Manual" under the CH32V307EVT\EVT\PUB folder.
 */
#include "string.h"
#include "debug.h"
#include "wchnet.h"
#include "eth_driver.h"



/* CAN Mode Definition */
#define TX_MODE   0
#define RX_MODE   1

/* Frame Format Definition */
#define Standard_Frame   0
#define Extended_Frame   1

/* CAN Communication Mode Selection */
#define CAN_MODE   TX_MODE
//#define CAN_MODE   RX_MODE

/* Frame Formate Selection */
#define Frame_Format   Standard_Frame
//#define Frame_Format   Extended_Frame

/*********************************************************************
 * @fn      CAN_Mode_Init
 *
 * @brief   Initializes CAN communication test mode.
 *          Bps =Fpclk1/((tpb1+1+tbs2+1+1)*brp)
 *
 * @param   tsjw - CAN synchronisation jump width.
 *          tbs2 - CAN time quantum in bit segment 1.
 *          tbs1 - CAN time quantum in bit segment 2.
 *          brp - Specifies the length of a time quantum.
 *          mode - Test mode.
 *            CAN_Mode_Normal.
 *            CAN_Mode_LoopBack.
 *            CAN_Mode_Silent.
 *            CAN_Mode_Silent_LoopBack.
 *
 * @return  none
 */
void CAN_Mode_Init(u8 tsjw, u8 tbs2, u8 tbs1, u16 brp, u8 mode) {
    GPIO_InitTypeDef GPIO_InitSturcture = {0};
    CAN_InitTypeDef CAN_InitSturcture = {0};
    CAN_FilterInitTypeDef CAN_FilterInitSturcture = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);

    GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitSturcture.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitSturcture);

    GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitSturcture);

    CAN_InitSturcture.CAN_TTCM = DISABLE;
    CAN_InitSturcture.CAN_ABOM = DISABLE;
    CAN_InitSturcture.CAN_AWUM = DISABLE;
    CAN_InitSturcture.CAN_NART = ENABLE;
    CAN_InitSturcture.CAN_RFLM = DISABLE;
    CAN_InitSturcture.CAN_TXFP = DISABLE;
    CAN_InitSturcture.CAN_Mode = mode;
    CAN_InitSturcture.CAN_SJW = tsjw;
    CAN_InitSturcture.CAN_BS1 = tbs1;
    CAN_InitSturcture.CAN_BS2 = tbs2;
    CAN_InitSturcture.CAN_Prescaler = brp;
    CAN_Init(CAN1, &CAN_InitSturcture);

    CAN_FilterInitSturcture.CAN_FilterNumber = 0;

#if (Frame_Format == Standard_Frame)
/* identifier/mask mode, One 32-bit filter, StdId: 0x317 */
    CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitSturcture.CAN_FilterIdHigh = 0x62E0;
    CAN_FilterInitSturcture.CAN_FilterIdLow = 0;
    CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0xFFE0;
    CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0x0006;

#elif (Frame_Format == Extended_Frame)
    /* identifier/mask mode, One 32-bit filter, ExtId: 0x12124567 */
    CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitSturcture.CAN_FilterIdHigh = 0x9092;
    CAN_FilterInitSturcture.CAN_FilterIdLow = 0x2B3C;
    CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0xFFFF;
    CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0xFFFE;

#endif

    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    CAN_FilterInitSturcture.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&CAN_FilterInitSturcture);
}

/*********************************************************************
 * @fn      CAN_Send_Msg
 *
 * @brief   CAN Transmit function.
 *
 * @param   msg - Transmit data buffer.
 *          len - Data length.
 *
 * @return  0 - Send successful.
 *          1 - Send failed.
 */
u8 CAN_Send_Msg(u8 *msg, u8 len) {
    u8 mbox;
    u16 i = 0;

    CanTxMsg CanTxStructure;

#if (Frame_Format == Standard_Frame)
    CanTxStructure.StdId = 0x317;
    CanTxStructure.IDE = CAN_Id_Standard;

#elif (Frame_Format == Extended_Frame)
    CanTxStructure.ExtId = 0x12124567;
    CanTxStructure.IDE = CAN_Id_Extended;

#endif

    CanTxStructure.RTR = CAN_RTR_Data;
    CanTxStructure.DLC = len;

    for (i = 0; i < len; i++) {
        CanTxStructure.Data[i] = msg[i];
    }

    mbox = CAN_Transmit(CAN1, &CanTxStructure);
    i = 0;

    while ((CAN_TransmitStatus(CAN1, mbox) != CAN_TxStatus_Ok) && (i < 0xFFF)) {
        i++;
    }

    if (i == 0xFFF) {
        return 1;
    } else {
        return 0;
    }
}

/*********************************************************************
 * @fn      CAN_Receive_Msg
 *
 * @brief   CAN Receive function.
 *
 * @param   buf - Receive data buffer.
 *          len - Data length.
 *
 * @return  CanRxStructure.DLC - Receive data length.
 */
u8 CAN_Receive_Msg(u8 *buf) {
    u8 i;

    CanRxMsg CanRxStructure;

    if (CAN_MessagePending(CAN1, CAN_FIFO0) == 0) {
        return 0;
    }

    CAN_Receive(CAN1, CAN_FIFO0, &CanRxStructure);

    for (i = 0; i < 8; i++) {
        buf[i] = CanRxStructure.Data[i];
    }

    return CanRxStructure.DLC;
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
 * @fn      TIM2_Init
 *
 * @brief   Initializes TIM2.
 *
 * @return  none
 */
void TIM2_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = SystemCoreClock / 1000000 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = WCHNETTIMERPERIOD * 1000 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM2, ENABLE);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    NVIC_EnableIRQ(TIM2_IRQn);
}

/*********************************************************************
 * @fn      TIM3_Init
 *
 * @brief   Initializes TIM3.
 *
 * @return  none
 */
void TIM3_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = SystemCoreClock / 1000000 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = WCHNETTIMERPERIOD * 1000 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM3, ENABLE);
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    NVIC_EnableIRQ(TIM3_IRQn);
}

void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

static int timex = 0;

/*********************************************************************
 * @fn      TIM3_IRQHandler
 *
 * @brief   This function handles TIM3 exception.
 *
 * @return  none
 */
void TIM3_IRQHandler(void) {
    timex++;
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
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
        p = arg;
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

void do_CAN_task() {
    u8 i;
    u8 cnt = 1;
    u8 px;
    u8 pxbuf[8];
    static int next_time = 0;
#if (CAN_MODE == TX_MODE)
    if (timex > next_time) {
        next_time = timex + 100;
        for (i = 0; i < 8; i++) {
            pxbuf[i] = cnt + i;
        }
        px = CAN_Send_Msg(pxbuf, 8);
        if (px) {
            printf("Send Failed\r\n");
        } else {
            printf("Send Success\r\n");
            printf("Send Data:\r\n");
            for (i = 0; i < 8; i++) {
                printf("%02x\r\n", pxbuf[i]);
            }
        }
    }
#elif (CAN_MODE == RX_MODE)
    px = CAN_Receive_Msg( pxbuf );
        if( px )
        {
            printf( "Receive Data:\r\n" );
            for(i=0; i<8; i++)
            {
                printf( "%02x\r\n", pxbuf[i] );
            }
        }
#endif
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program
 *
 * @return  none
 */
int main(void) {
    u8 i;

    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);                                    //USART initialize
    printf("\r\nTcpServer Test\r\n");
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());

#if (CAN_MODE == TX_MODE)
    printf("Tx Mode\r\n");
#elif (CAN_MODE == RX_MODE)
    printf( "Rx Mode\r\n" );
#endif
/* Bps = 333Kbps */
    CAN_Mode_Init(CAN_SJW_1tq, CAN_BS2_5tq, CAN_BS1_6tq, 12, CAN_Mode_Normal);

    printf("net version:%x\r\n", WCHNET_GetVer());
    if (WCHNET_LIB_VER != WCHNET_GetVer()) {
        printf("version error.\r\n");
    }
    WCHNET_GetMacAddr(MACAddr);                                   //get the chip MAC address
    printf("mac addr:");
    for (i = 0; i < 6; i++)
        printf("%x ", MACAddr[i]);
    printf("\r\n");
    TIM2_Init();
    TIM3_Init();
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

        do_CAN_task();
    }
}


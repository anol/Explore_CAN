//
// Created by aeols on 02.03.2023.
//

#include "Handle_CAN.h"


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

Handle_CAN::Handle_CAN(Handle_MCU &CH32) :
        use_CH32(CH32),
        tsjw(CAN_SJW_1tq),
        tbs2(CAN_BS2_5tq),
        tbs1(CAN_BS1_6tq),
        brp(4) {}

void Handle_CAN::initialize() {
    setup_CAN(CAN_Mode_Normal);
}

void Handle_CAN::print_info() const {
#if (CAN_MODE == TX_MODE)
    printf("Tx Mode\r\n");
#elif (CAN_MODE == RX_MODE)
    printf( "Rx Mode\r\n" );
#endif

    int pclk = 96000000 / 2;
    int bps = pclk / (brp * (tbs1 + 1 + tbs2 + 1 + 1));
    printf("tsjw:%8d\r\n", tsjw);
    printf("tbs2:%8d\r\n", tbs2);
    printf("tbs1:%8d\r\n", tbs1);
    printf("brp: %8d\r\n", brp);
    printf("bps= %8d\r\n", bps);

}

void Handle_CAN::task() {
    u8 i;
    u8 cnt = 1;
    u8 px;
    u8 pxbuf[8];

#if (Frame_Format == Standard_Frame)
    u32 id = 0x317;
    bool extended = false;
#elif (Frame_Format == Extended_Frame)
    u32 id = 0x12124567;
    bool extended = true;
#endif

#if (CAN_MODE == TX_MODE)
    auto timex = use_CH32.get_timex();
    if (timex > the_next_time) {
        the_next_time = timex + 100;
        for (i = 0; i < 8; i++) {
            pxbuf[i] = cnt + i;
        }
        px = transmit(pxbuf, 8, id, extended);
        if (px) {
            printf("Send Failed\r\n");
        } else {
            printf("Send Data: ");
            for (i = 0; i < 8; i++) {
                printf("%02x ", pxbuf[i]);
            }
            printf("\r\n");
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

u8 Handle_CAN::transmit(u8 *msg, u8 len, u32 id, bool extended) {
    u8 mbox;
    u16 i = 0;

    CanTxMsg CanTxStructure;
    CanTxStructure.IDE = extended ? CAN_Id_Extended : CAN_Id_Standard;
    CanTxStructure.StdId = id;
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

u8 Handle_CAN::receive(u8 *buf) {
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

void Handle_CAN::setup_CAN(u8 mode) {
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

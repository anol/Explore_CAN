//
// Created by aeols on 02.03.2023.
//


#include "Handle_MCU.h"

uint32_t Handle_MCU::the_timex{};

extern "C" void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
extern "C" void TIM3_IRQHandler(void) {
    Handle_MCU::the_timex++;
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}

void Handle_MCU::initialize() {
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);                                    //USART initialize
    the_clock = SystemCoreClock;
    the_chip_id = DBGMCU_GetCHIPID();
    setup_TIM2();
    setup_TIM3();
}

void Handle_MCU::print_info() const {
    printf("SystemClk:%lu\r\n", the_clock);
    printf("ChipID:%08lx\r\n", the_chip_id);
}

void Handle_MCU::setup_TIM2() {
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

void Handle_MCU::setup_TIM3() {
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

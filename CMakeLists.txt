cmake_minimum_required(VERSION 3.24)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CROSSCOMPILING 1)
set(CMAKE_SYSTEM_PROCESSOR riscv)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER_ID riscv)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_COMPILER_ID riscv)
set(CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs")

set(TOOLCHAIN_HOME "C:/MounRiver/MRS_Community/toolchain/RISC-V Embedded GCC")
set(OBJCOPY ${TOOLCHAIN_HOME}/bin/riscv-none-embed-objcopy.exe)

project(Explore_CAN CXX C ASM)

add_executable(${PROJECT_NAME}.elf
        src/main.cpp
        src/Explore_CAN.cpp
        src/Handle_LAN.cpp
        src/Handle_LAN.h
        src/Handle_CAN.cpp
        src/Handle_CAN.h
        src/Handle_MCU.cpp
        src/Handle_MCU.h
        )

target_sources(${PROJECT_NAME}.elf PUBLIC
        src/CH32V30X/ch32v30x_conf.h
        src/CH32V30X/ch32v30x_it.c
        src/CH32V30X/ch32v30x_it.h
        src/CH32V30X/Core/core_riscv.c
        src/CH32V30X/Core/core_riscv.h
        src/CH32V30X/Debug/debug.c
        src/CH32V30X/Debug/debug.h
        src/CH32V30X/net_config.h
        src/CH32V30X/NetLib/eth_driver.c
        src/CH32V30X/NetLib/eth_driver.h
        src/CH32V30X/NetLib/wchnet.h
        src/CH32V30X/Peripheral/inc/ch32v30x.h
        src/CH32V30X/Peripheral/inc/ch32v30x_adc.h
        src/CH32V30X/Peripheral/inc/ch32v30x_bkp.h
        src/CH32V30X/Peripheral/inc/ch32v30x_can.h
        src/CH32V30X/Peripheral/inc/ch32v30x_crc.h
        src/CH32V30X/Peripheral/inc/ch32v30x_dac.h
        src/CH32V30X/Peripheral/inc/ch32v30x_dbgmcu.h
        src/CH32V30X/Peripheral/inc/ch32v30x_dma.h
        src/CH32V30X/Peripheral/inc/ch32v30x_dvp.h
        src/CH32V30X/Peripheral/inc/ch32v30x_eth.h
        src/CH32V30X/Peripheral/inc/ch32v30x_exti.h
        src/CH32V30X/Peripheral/inc/ch32v30x_flash.h
        src/CH32V30X/Peripheral/inc/ch32v30x_fsmc.h
        src/CH32V30X/Peripheral/inc/ch32v30x_gpio.h
        src/CH32V30X/Peripheral/inc/ch32v30x_i2c.h
        src/CH32V30X/Peripheral/inc/ch32v30x_iwdg.h
        src/CH32V30X/Peripheral/inc/ch32v30x_misc.h
        src/CH32V30X/Peripheral/inc/ch32v30x_opa.h
        src/CH32V30X/Peripheral/inc/ch32v30x_pwr.h
        src/CH32V30X/Peripheral/inc/ch32v30x_rcc.h
        src/CH32V30X/Peripheral/inc/ch32v30x_rng.h
        src/CH32V30X/Peripheral/inc/ch32v30x_rtc.h
        src/CH32V30X/Peripheral/inc/ch32v30x_sdio.h
        src/CH32V30X/Peripheral/inc/ch32v30x_spi.h
        src/CH32V30X/Peripheral/inc/ch32v30x_tim.h
        src/CH32V30X/Peripheral/inc/ch32v30x_usart.h
        src/CH32V30X/Peripheral/inc/ch32v30x_wwdg.h
        src/CH32V30X/Peripheral/src/ch32v30x_adc.c
        src/CH32V30X/Peripheral/src/ch32v30x_bkp.c
        src/CH32V30X/Peripheral/src/ch32v30x_can.c
        src/CH32V30X/Peripheral/src/ch32v30x_crc.c
        src/CH32V30X/Peripheral/src/ch32v30x_dac.c
        src/CH32V30X/Peripheral/src/ch32v30x_dbgmcu.c
        src/CH32V30X/Peripheral/src/ch32v30x_dma.c
        src/CH32V30X/Peripheral/src/ch32v30x_dvp.c
        src/CH32V30X/Peripheral/src/ch32v30x_eth.c
        src/CH32V30X/Peripheral/src/ch32v30x_exti.c
        src/CH32V30X/Peripheral/src/ch32v30x_flash.c
        src/CH32V30X/Peripheral/src/ch32v30x_fsmc.c
        src/CH32V30X/Peripheral/src/ch32v30x_gpio.c
        src/CH32V30X/Peripheral/src/ch32v30x_i2c.c
        src/CH32V30X/Peripheral/src/ch32v30x_iwdg.c
        src/CH32V30X/Peripheral/src/ch32v30x_misc.c
        src/CH32V30X/Peripheral/src/ch32v30x_opa.c
        src/CH32V30X/Peripheral/src/ch32v30x_pwr.c
        src/CH32V30X/Peripheral/src/ch32v30x_rcc.c
        src/CH32V30X/Peripheral/src/ch32v30x_rng.c
        src/CH32V30X/Peripheral/src/ch32v30x_rtc.c
        src/CH32V30X/Peripheral/src/ch32v30x_sdio.c
        src/CH32V30X/Peripheral/src/ch32v30x_spi.c
        src/CH32V30X/Peripheral/src/ch32v30x_tim.c
        src/CH32V30X/Peripheral/src/ch32v30x_usart.c
        src/CH32V30X/Peripheral/src/ch32v30x_wwdg.c
        src/CH32V30X/startup_ch32v30x_D8C.S
        src/CH32V30X/system_ch32v30x.c
        src/CH32V30X/system_ch32v30x.h
        )

add_custom_target(${PROJECT_NAME}.hex ALL)

add_dependencies(${PROJECT_NAME}.hex ${PROJECT_NAME}.elf)

add_custom_command(TARGET ${PROJECT_NAME}.hex POST_BUILD
        COMMAND ${OBJCOPY}
        -O ihex
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.elf
        ${PROJECT_SOURCE_DIR}/bin/${PROJECT_NAME}.hex
        )

include_directories(${PROJECT_NAME}.elf
        src
        src/CH32V30X/Core
        src/CH32V30X/Debug
        src/CH32V30X/NetLib
        src/CH32V30X/Peripheral/inc
        )

target_link_options(${PROJECT_NAME}.elf PUBLIC
        -nostartfiles
        -fno-exceptions
        -Xlinker
        -T ${CMAKE_SOURCE_DIR}/src/CH32V30X/CH32V30X.ld
        -Wl,-Map=${TARGET}.map
        )

target_link_libraries(${PROJECT_NAME}.elf
        ${CMAKE_SOURCE_DIR}/src/CH32V30X/NetLib/libwchnet.a
        ${CMAKE_SOURCE_DIR}/src/CH32V30X/NetLib/libwchnet_float.a
        )

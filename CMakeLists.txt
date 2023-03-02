cmake_minimum_required(VERSION 3.24)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CROSSCOMPILING 1)
set(CMAKE_SYSTEM_PROCESSOR riscv)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER_ID riscv)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_COMPILER_ID riscv)
set(CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs")

set(TOOLCHAIN_HOME "C:/MounRiver/MRS_Community/toolchain/RISC-V Embedded GCC" )
set(OBJCOPY ${TOOLCHAIN_HOME}/bin/riscv-none-embed-objcopy.exe)

project(Explore_CAN CXX C ASM)

add_executable(${PROJECT_NAME}.elf
        src/ch32v30x_conf.h
        src/ch32v30x_it.c
        src/ch32v30x_it.h
        src/main.cpp
        src/net_config.h
        src/system_ch32v30x.c
        src/system_ch32v30x.h
        src/startup_ch32v30x_D8C.S

        src/NetLib/wchnet.h
        src/NetLib/eth_driver.h
        src/NetLib/eth_driver.c
        src/Debug/debug.c
        src/Debug/debug.h
        src/Core/core_riscv.c
        src/Core/core_riscv.h
        src/Peripheral/inc/ch32v30x.h
        src/Peripheral/inc/ch32v30x_adc.h src/Peripheral/inc/ch32v30x_bkp.h src/Peripheral/inc/ch32v30x_can.h
        src/Peripheral/inc/ch32v30x_crc.h src/Peripheral/inc/ch32v30x_dac.h src/Peripheral/inc/ch32v30x_dbgmcu.h
        src/Peripheral/inc/ch32v30x_dma.h src/Peripheral/inc/ch32v30x_dvp.h src/Peripheral/inc/ch32v30x_eth.h
        src/Peripheral/inc/ch32v30x_exti.h src/Peripheral/inc/ch32v30x_flash.h src/Peripheral/inc/ch32v30x_fsmc.h
        src/Peripheral/inc/ch32v30x_gpio.h src/Peripheral/inc/ch32v30x_i2c.h src/Peripheral/inc/ch32v30x_iwdg.h
        src/Peripheral/inc/ch32v30x_misc.h src/Peripheral/inc/ch32v30x_opa.h src/Peripheral/inc/ch32v30x_pwr.h
        src/Peripheral/inc/ch32v30x_rcc.h src/Peripheral/inc/ch32v30x_rng.h src/Peripheral/inc/ch32v30x_rtc.h
        src/Peripheral/inc/ch32v30x_sdio.h src/Peripheral/inc/ch32v30x_spi.h src/Peripheral/inc/ch32v30x_tim.h
        src/Peripheral/inc/ch32v30x_usart.h src/Peripheral/inc/ch32v30x_wwdg.h
        src/Peripheral/src/ch32v30x_adc.c src/Peripheral/src/ch32v30x_bkp.c src/Peripheral/src/ch32v30x_can.c
        src/Peripheral/src/ch32v30x_crc.c src/Peripheral/src/ch32v30x_dac.c src/Peripheral/src/ch32v30x_dbgmcu.c
        src/Peripheral/src/ch32v30x_dma.c src/Peripheral/src/ch32v30x_dvp.c src/Peripheral/src/ch32v30x_eth.c
        src/Peripheral/src/ch32v30x_exti.c src/Peripheral/src/ch32v30x_flash.c src/Peripheral/src/ch32v30x_fsmc.c
        src/Peripheral/src/ch32v30x_gpio.c src/Peripheral/src/ch32v30x_i2c.c src/Peripheral/src/ch32v30x_iwdg.c
        src/Peripheral/src/ch32v30x_misc.c src/Peripheral/src/ch32v30x_opa.c src/Peripheral/src/ch32v30x_pwr.c
        src/Peripheral/src/ch32v30x_rcc.c src/Peripheral/src/ch32v30x_rng.c src/Peripheral/src/ch32v30x_rtc.c
        src/Peripheral/src/ch32v30x_sdio.c src/Peripheral/src/ch32v30x_spi.c src/Peripheral/src/ch32v30x_tim.c
        src/Peripheral/src/ch32v30x_usart.c src/Peripheral/src/ch32v30x_wwdg.c

        src/Explore_CAN.cpp src/CAN_handler.cpp src/CAN_handler.h src/LAN_handler.cpp src/LAN_handler.h src/CH32_platform.cpp src/CH32_platform.h)

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
        src/Core
        src/Debug
        src/NetLib
        src/Peripheral/inc
        )

target_link_options(${PROJECT_NAME}.elf PUBLIC
        -nostartfiles
        -fno-exceptions
        -Xlinker
#        --defsym=__stack_size=${STACK_SIZE}
        -T ${CMAKE_SOURCE_DIR}/src/linker.ld
        -Wl,-Map=${TARGET}.map
        )

target_link_libraries(${PROJECT_NAME}.elf
        ${CMAKE_SOURCE_DIR}/src/NetLib/libwchnet.a
        ${CMAKE_SOURCE_DIR}/src/NetLib/libwchnet_float.a
        )

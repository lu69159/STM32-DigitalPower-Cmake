cmake_minimum_required(VERSION 3.20)
project(DigitalPowerCmake C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(TARGET_NAME ${CMAKE_PROJECT_NAME}.elf)

set(CPU_FLAGS -mcpu=cortex-m4 -mthumb)
set(FPU_FLAGS -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

add_compile_options(
    ${CPU_FLAGS} ${FPU_FLAGS}
    $<$<CONFIG:Debug>:-Og>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Debug>:-DDEBUG>
    $<$<CONFIG:Release>:-Os>
    -ffunction-sections -fdata-sections
)

add_link_options(
    ${CPU_FLAGS} ${FPU_FLAGS}
    -T${CMAKE_SOURCE_DIR}/STM32G474RETX_FLASH.ld
    -Wl,-Map=${CMAKE_PROJECT_NAME}.map,--cref
    -Wl,--gc-sections
    --specs=nano.specs
    -Wl,--start-group -lc -lm -Wl,--end-group
)

add_definitions(-DUSE_HAL_DRIVER -DSTM32G474xx)

set(INC_DIRS
    Core/Inc
    Drivers/STM32G4xx_HAL_Driver/Inc
    Drivers/STM32G4xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32G4xx/Include
    Drivers/CMSIS/Include
)

file(GLOB_RECURSE SOURCES
    Core/Src/*.c
    Drivers/STM32G4xx_HAL_Driver/Src/*.c
)

set(STARTUP_FILE Core/Startup/startup_stm32g474retx.s)

add_executable(${TARGET_NAME} ${SOURCES} ${STARTUP_FILE})

target_include_directories(${TARGET_NAME} PRIVATE ${INC_DIRS})

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${TARGET_NAME} ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET_NAME} ${CMAKE_PROJECT_NAME}.bin
    COMMENT "Generating HEX and BIN formats"
)

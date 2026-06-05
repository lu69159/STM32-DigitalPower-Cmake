set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m4)

# Find Ninja (build tool bundled with STM32CubeCLT)
file(GLOB NINJA_CANDIDATES
    $ENV{LOCALAPPDATA}/stm32cube/bundles/ninja/*/bin/ninja.exe
    $ENV{LOCALAPPDATA}/STM32Cube/bundles/ninja/*/bin/ninja.exe
)
if(NINJA_CANDIDATES)
    list(GET NINJA_CANDIDATES 0 CMAKE_MAKE_PROGRAM)
    set(CMAKE_MAKE_PROGRAM ${CMAKE_MAKE_PROGRAM} CACHE FILEPATH "Path to Ninja")
    get_filename_component(NINJA_DIR ${CMAKE_MAKE_PROGRAM} DIRECTORY)
    message(STATUS "Found Ninja: ${CMAKE_MAKE_PROGRAM}")
endif()

# Find arm-none-eabi-gcc
file(GLOB GCC_CANDIDATES
    $ENV{LOCALAPPDATA}/stm32cube/bundles/gnu-tools-for-stm32/*/bin/arm-none-eabi-gcc.exe
    $ENV{LOCALAPPDATA}/STM32Cube/bundles/gnu-tools-for-stm32/*/bin/arm-none-eabi-gcc.exe
    C:/ST/STM32CubeCLT/*/GNU-tools-for-STM32/bin/arm-none-eabi-gcc.exe
    C:/ST/STM32CubeCLT/GNU-tools-for-STM32/bin/arm-none-eabi-gcc.exe
)

if(GCC_CANDIDATES)
    list(GET GCC_CANDIDATES 0 ARM_NONE_EABI_GCC)
    set(CMAKE_C_COMPILER ${ARM_NONE_EABI_GCC})
    set(CMAKE_ASM_COMPILER ${ARM_NONE_EABI_GCC})
    message(STATUS "Found ARM GCC: ${ARM_NONE_EABI_GCC}")
else()
    find_program(ARM_NONE_EABI_GCC arm-none-eabi-gcc)
    if(ARM_NONE_EABI_GCC)
        set(CMAKE_C_COMPILER ${ARM_NONE_EABI_GCC})
        set(CMAKE_ASM_COMPILER ${ARM_NONE_EABI_GCC})
        message(STATUS "Found ARM GCC (from PATH): ${ARM_NONE_EABI_GCC}")
    else()
        message(FATAL_ERROR
            "arm-none-eabi-gcc not found.\n"
            "Install STM32CubeCLT or set ARM_GCC_PATH environment variable.\n"
            "Searched in: $ENV{LOCALAPPDATA}/stm32cube/bundles/gnu-tools-for-stm32/"
        )
    endif()
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

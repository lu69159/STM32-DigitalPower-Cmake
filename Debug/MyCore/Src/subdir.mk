################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../MyCore/Src/font.c \
../MyCore/Src/oled.c \
../MyCore/Src/task.c 

OBJS += \
./MyCore/Src/font.o \
./MyCore/Src/oled.o \
./MyCore/Src/task.o 

C_DEPS += \
./MyCore/Src/font.d \
./MyCore/Src/oled.d \
./MyCore/Src/task.d 


# Each subdirectory must supply rules for building sources it contributes
MyCore/Src/%.o MyCore/Src/%.su MyCore/Src/%.cyclo: ../MyCore/Src/%.c MyCore/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../MyCore/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-MyCore-2f-Src

clean-MyCore-2f-Src:
	-$(RM) ./MyCore/Src/font.cyclo ./MyCore/Src/font.d ./MyCore/Src/font.o ./MyCore/Src/font.su ./MyCore/Src/oled.cyclo ./MyCore/Src/oled.d ./MyCore/Src/oled.o ./MyCore/Src/oled.su ./MyCore/Src/task.cyclo ./MyCore/Src/task.d ./MyCore/Src/task.o ./MyCore/Src/task.su

.PHONY: clean-MyCore-2f-Src


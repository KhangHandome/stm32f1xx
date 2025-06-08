################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lib/src/PID.c \
../lib/src/hal_dma.c \
../lib/src/hal_usart.c 

OBJS += \
./lib/src/PID.o \
./lib/src/hal_dma.o \
./lib/src/hal_usart.o 

C_DEPS += \
./lib/src/PID.d \
./lib/src/hal_dma.d \
./lib/src/hal_usart.d 


# Each subdirectory must supply rules for building sources it contributes
lib/src/%.o lib/src/%.su lib/src/%.cyclo: ../lib/src/%.c lib/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/My Document/stm32f1xx/coding/PID_esp8266_stm32/lib/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-lib-2f-src

clean-lib-2f-src:
	-$(RM) ./lib/src/PID.cyclo ./lib/src/PID.d ./lib/src/PID.o ./lib/src/PID.su ./lib/src/hal_dma.cyclo ./lib/src/hal_dma.d ./lib/src/hal_dma.o ./lib/src/hal_dma.su ./lib/src/hal_usart.cyclo ./lib/src/hal_usart.d ./lib/src/hal_usart.o ./lib/src/hal_usart.su

.PHONY: clean-lib-2f-src


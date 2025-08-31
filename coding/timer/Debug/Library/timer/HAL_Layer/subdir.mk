################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Library/timer/HAL_Layer/HAL_Timer.c 

OBJS += \
./Library/timer/HAL_Layer/HAL_Timer.o 

C_DEPS += \
./Library/timer/HAL_Layer/HAL_Timer.d 


# Each subdirectory must supply rules for building sources it contributes
Library/timer/HAL_Layer/%.o Library/timer/HAL_Layer/%.su Library/timer/HAL_Layer/%.cyclo: ../Library/timer/HAL_Layer/%.c Library/timer/HAL_Layer/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"/home/khangmt/Documents/GitWork/stm32f1xx/coding/timer/Library/timer/HAL_Layer" -I"/home/khangmt/Documents/GitWork/stm32f1xx/coding/timer/Library/timer/MID_Layer" -I"/home/khangmt/Documents/GitWork/stm32f1xx/coding/timer/Library/timer/App_Layer" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Library-2f-timer-2f-HAL_Layer

clean-Library-2f-timer-2f-HAL_Layer:
	-$(RM) ./Library/timer/HAL_Layer/HAL_Timer.cyclo ./Library/timer/HAL_Layer/HAL_Timer.d ./Library/timer/HAL_Layer/HAL_Timer.o ./Library/timer/HAL_Layer/HAL_Timer.su

.PHONY: clean-Library-2f-timer-2f-HAL_Layer


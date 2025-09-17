################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Library/timer/MID_Layer/MID_Timer.c 

OBJS += \
./Library/timer/MID_Layer/MID_Timer.o 

C_DEPS += \
./Library/timer/MID_Layer/MID_Timer.d 


# Each subdirectory must supply rules for building sources it contributes
Library/timer/MID_Layer/%.o Library/timer/MID_Layer/%.su Library/timer/MID_Layer/%.cyclo: ../Library/timer/MID_Layer/%.c Library/timer/MID_Layer/subdir.mk
<<<<<<< HEAD
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"/media/khangmt/6008FB9808FB6C02/Users/maith/Documents/GitWork/stm32f1xx/coding/timer/Library/timer/HAL_Layer" -I"/media/khangmt/6008FB9808FB6C02/Users/maith/Documents/GitWork/stm32f1xx/coding/timer/Library/timer/MID_Layer" -I"/media/khangmt/6008FB9808FB6C02/Users/maith/Documents/GitWork/stm32f1xx/coding/timer/Library/timer/App_Layer" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
=======
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"/home/khangmt/Documents/GitWork/stm32f1xx/coding/timer/Library/timer/HAL_Layer" -I"/home/khangmt/Documents/GitWork/stm32f1xx/coding/timer/Library/timer/MID_Layer" -I"/home/khangmt/Documents/GitWork/stm32f1xx/coding/timer/Library/timer/App_Layer" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
>>>>>>> 577c4389665029f4d923455370503e00bef7ac2d

clean: clean-Library-2f-timer-2f-MID_Layer

clean-Library-2f-timer-2f-MID_Layer:
	-$(RM) ./Library/timer/MID_Layer/MID_Timer.cyclo ./Library/timer/MID_Layer/MID_Timer.d ./Library/timer/MID_Layer/MID_Timer.o ./Library/timer/MID_Layer/MID_Timer.su

.PHONY: clean-Library-2f-timer-2f-MID_Layer


################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../EXT_Library/Src/DRV_Lora.c 

C_DEPS += \
./EXT_Library/Src/DRV_Lora.d 

OBJS += \
./EXT_Library/Src/DRV_Lora.o 


# Each subdirectory must supply rules for building sources it contributes
EXT_Library/Src/%.o EXT_Library/Src/%.su EXT_Library/Src/%.cyclo: ../EXT_Library/Src/%.c EXT_Library/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"C:/Users/maith/Documents/GitWork/stm32f1xx/coding/LoraDriver/EXT_Library/Inc" -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-EXT_Library-2f-Src

clean-EXT_Library-2f-Src:
	-$(RM) ./EXT_Library/Src/DRV_Lora.cyclo ./EXT_Library/Src/DRV_Lora.d ./EXT_Library/Src/DRV_Lora.o ./EXT_Library/Src/DRV_Lora.su

.PHONY: clean-EXT_Library-2f-Src


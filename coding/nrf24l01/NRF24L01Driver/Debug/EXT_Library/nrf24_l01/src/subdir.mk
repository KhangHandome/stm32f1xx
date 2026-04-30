################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../EXT_Library/nrf24_l01/src/DRV_NRF24L01.c 

OBJS += \
./EXT_Library/nrf24_l01/src/DRV_NRF24L01.o 

C_DEPS += \
./EXT_Library/nrf24_l01/src/DRV_NRF24L01.d 


# Each subdirectory must supply rules for building sources it contributes
EXT_Library/nrf24_l01/src/%.o EXT_Library/nrf24_l01/src/%.su EXT_Library/nrf24_l01/src/%.cyclo: ../EXT_Library/nrf24_l01/src/%.c EXT_Library/nrf24_l01/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"C:/Users/maith/Documents/GitWork/stm32f1xx/coding/nrf24l01/NRF24L01Driver/EXT_Library/nrf24_l01/inc" -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-EXT_Library-2f-nrf24_l01-2f-src

clean-EXT_Library-2f-nrf24_l01-2f-src:
	-$(RM) ./EXT_Library/nrf24_l01/src/DRV_NRF24L01.cyclo ./EXT_Library/nrf24_l01/src/DRV_NRF24L01.d ./EXT_Library/nrf24_l01/src/DRV_NRF24L01.o ./EXT_Library/nrf24_l01/src/DRV_NRF24L01.su

.PHONY: clean-EXT_Library-2f-nrf24_l01-2f-src


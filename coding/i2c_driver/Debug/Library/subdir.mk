################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Library/hal_i2c.c 

OBJS += \
./Library/hal_i2c.o 

C_DEPS += \
./Library/hal_i2c.d 


# Each subdirectory must supply rules for building sources it contributes
Library/%.o Library/%.su Library/%.cyclo: ../Library/%.c Library/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/maith/Documents/Learning/learn_master/Develop Application Control From The Computer/Midterm/Code/Stm32/i2c_driver/Library" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Library

clean-Library:
	-$(RM) ./Library/hal_i2c.cyclo ./Library/hal_i2c.d ./Library/hal_i2c.o ./Library/hal_i2c.su

.PHONY: clean-Library


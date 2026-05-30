################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../STM32CubeIDE/Application/User/Core/syscalls.c \
../STM32CubeIDE/Application/User/Core/sysmem.c 

OBJS += \
./STM32CubeIDE/Application/User/Core/syscalls.o \
./STM32CubeIDE/Application/User/Core/sysmem.o 

C_DEPS += \
./STM32CubeIDE/Application/User/Core/syscalls.d \
./STM32CubeIDE/Application/User/Core/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
STM32CubeIDE/Application/User/Core/%.o STM32CubeIDE/Application/User/Core/%.su STM32CubeIDE/Application/User/Core/%.cyclo: ../STM32CubeIDE/Application/User/Core/%.c STM32CubeIDE/Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-STM32CubeIDE-2f-Application-2f-User-2f-Core

clean-STM32CubeIDE-2f-Application-2f-User-2f-Core:
	-$(RM) ./STM32CubeIDE/Application/User/Core/syscalls.cyclo ./STM32CubeIDE/Application/User/Core/syscalls.d ./STM32CubeIDE/Application/User/Core/syscalls.o ./STM32CubeIDE/Application/User/Core/syscalls.su ./STM32CubeIDE/Application/User/Core/sysmem.cyclo ./STM32CubeIDE/Application/User/Core/sysmem.d ./STM32CubeIDE/Application/User/Core/sysmem.o ./STM32CubeIDE/Application/User/Core/sysmem.su

.PHONY: clean-STM32CubeIDE-2f-Application-2f-User-2f-Core


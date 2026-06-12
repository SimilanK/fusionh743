################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../control.c \
../control_data.c \
../ert_main.c \
../rt_nonfinite.c 

OBJS += \
./control.o \
./control_data.o \
./ert_main.o \
./rt_nonfinite.o 

C_DEPS += \
./control.d \
./control_data.d \
./ert_main.d \
./rt_nonfinite.d 


# Each subdirectory must supply rules for building sources it contributes
%.o %.su %.cyclo: ../%.c subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean--2e-

clean--2e-:
	-$(RM) ./control.cyclo ./control.d ./control.o ./control.su ./control_data.cyclo ./control_data.d ./control_data.o ./control_data.su ./ert_main.cyclo ./ert_main.d ./ert_main.o ./ert_main.su ./rt_nonfinite.cyclo ./rt_nonfinite.d ./rt_nonfinite.o ./rt_nonfinite.su

.PHONY: clean--2e-


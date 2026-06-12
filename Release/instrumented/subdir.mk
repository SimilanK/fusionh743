################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../instrumented/code_profiling_utility_functions.c \
../instrumented/control.c \
../instrumented/control_data.c \
../instrumented/ert_main.c \
../instrumented/rt_nonfinite.c 

OBJS += \
./instrumented/code_profiling_utility_functions.o \
./instrumented/control.o \
./instrumented/control_data.o \
./instrumented/ert_main.o \
./instrumented/rt_nonfinite.o 

C_DEPS += \
./instrumented/code_profiling_utility_functions.d \
./instrumented/control.d \
./instrumented/control_data.d \
./instrumented/ert_main.d \
./instrumented/rt_nonfinite.d 


# Each subdirectory must supply rules for building sources it contributes
instrumented/%.o instrumented/%.su instrumented/%.cyclo: ../instrumented/%.c instrumented/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-instrumented

clean-instrumented:
	-$(RM) ./instrumented/code_profiling_utility_functions.cyclo ./instrumented/code_profiling_utility_functions.d ./instrumented/code_profiling_utility_functions.o ./instrumented/code_profiling_utility_functions.su ./instrumented/control.cyclo ./instrumented/control.d ./instrumented/control.o ./instrumented/control.su ./instrumented/control_data.cyclo ./instrumented/control_data.d ./instrumented/control_data.o ./instrumented/control_data.su ./instrumented/ert_main.cyclo ./instrumented/ert_main.d ./instrumented/ert_main.o ./instrumented/ert_main.su ./instrumented/rt_nonfinite.cyclo ./instrumented/rt_nonfinite.d ./instrumented/rt_nonfinite.o ./instrumented/rt_nonfinite.su

.PHONY: clean-instrumented


################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ICMSPI.c \
../Core/Src/Kalman2State.c \
../Core/Src/MS5607SPI.c \
../Core/Src/airbrake_app.c \
../Core/Src/ao_flight.c \
../Core/Src/control.c \
../Core/Src/control_data.c \
../Core/Src/lsm6dso32.c \
../Core/Src/main.c \
../Core/Src/mpl3115a2.c \
../Core/Src/pseudo_traj.c \
../Core/Src/rt_nonfinite.c \
../Core/Src/stm32h7xx_hal_msp.c \
../Core/Src/stm32h7xx_it.c \
../Core/Src/system_stm32h7xx.c 

OBJS += \
./Core/Src/ICMSPI.o \
./Core/Src/Kalman2State.o \
./Core/Src/MS5607SPI.o \
./Core/Src/airbrake_app.o \
./Core/Src/ao_flight.o \
./Core/Src/control.o \
./Core/Src/control_data.o \
./Core/Src/lsm6dso32.o \
./Core/Src/main.o \
./Core/Src/mpl3115a2.o \
./Core/Src/pseudo_traj.o \
./Core/Src/rt_nonfinite.o \
./Core/Src/stm32h7xx_hal_msp.o \
./Core/Src/stm32h7xx_it.o \
./Core/Src/system_stm32h7xx.o 

C_DEPS += \
./Core/Src/ICMSPI.d \
./Core/Src/Kalman2State.d \
./Core/Src/MS5607SPI.d \
./Core/Src/airbrake_app.d \
./Core/Src/ao_flight.d \
./Core/Src/control.d \
./Core/Src/control_data.d \
./Core/Src/lsm6dso32.d \
./Core/Src/main.d \
./Core/Src/mpl3115a2.d \
./Core/Src/pseudo_traj.d \
./Core/Src/rt_nonfinite.d \
./Core/Src/stm32h7xx_hal_msp.d \
./Core/Src/stm32h7xx_it.d \
./Core/Src/system_stm32h7xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/ICMSPI.cyclo ./Core/Src/ICMSPI.d ./Core/Src/ICMSPI.o ./Core/Src/ICMSPI.su ./Core/Src/Kalman2State.cyclo ./Core/Src/Kalman2State.d ./Core/Src/Kalman2State.o ./Core/Src/Kalman2State.su ./Core/Src/MS5607SPI.cyclo ./Core/Src/MS5607SPI.d ./Core/Src/MS5607SPI.o ./Core/Src/MS5607SPI.su ./Core/Src/airbrake_app.cyclo ./Core/Src/airbrake_app.d ./Core/Src/airbrake_app.o ./Core/Src/airbrake_app.su ./Core/Src/ao_flight.cyclo ./Core/Src/ao_flight.d ./Core/Src/ao_flight.o ./Core/Src/ao_flight.su ./Core/Src/control.cyclo ./Core/Src/control.d ./Core/Src/control.o ./Core/Src/control.su ./Core/Src/control_data.cyclo ./Core/Src/control_data.d ./Core/Src/control_data.o ./Core/Src/control_data.su ./Core/Src/lsm6dso32.cyclo ./Core/Src/lsm6dso32.d ./Core/Src/lsm6dso32.o ./Core/Src/lsm6dso32.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/mpl3115a2.cyclo ./Core/Src/mpl3115a2.d ./Core/Src/mpl3115a2.o ./Core/Src/mpl3115a2.su ./Core/Src/pseudo_traj.cyclo ./Core/Src/pseudo_traj.d ./Core/Src/pseudo_traj.o ./Core/Src/pseudo_traj.su ./Core/Src/rt_nonfinite.cyclo ./Core/Src/rt_nonfinite.d ./Core/Src/rt_nonfinite.o ./Core/Src/rt_nonfinite.su ./Core/Src/stm32h7xx_hal_msp.cyclo ./Core/Src/stm32h7xx_hal_msp.d ./Core/Src/stm32h7xx_hal_msp.o ./Core/Src/stm32h7xx_hal_msp.su ./Core/Src/stm32h7xx_it.cyclo ./Core/Src/stm32h7xx_it.d ./Core/Src/stm32h7xx_it.o ./Core/Src/stm32h7xx_it.su ./Core/Src/system_stm32h7xx.cyclo ./Core/Src/system_stm32h7xx.d ./Core/Src/system_stm32h7xx.o ./Core/Src/system_stm32h7xx.su

.PHONY: clean-Core-2f-Src


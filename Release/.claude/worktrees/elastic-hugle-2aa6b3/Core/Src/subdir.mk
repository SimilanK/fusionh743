################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/ICMSPI.c \
../.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/Kalman2State.c \
../.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/MS5607SPI.c \
../.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/main.c \
../.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/stm32h7xx_hal_msp.c \
../.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/stm32h7xx_it.c \
../.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/system_stm32h7xx.c 

OBJS += \
./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/ICMSPI.o \
./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/Kalman2State.o \
./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/MS5607SPI.o \
./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/main.o \
./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/stm32h7xx_hal_msp.o \
./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/stm32h7xx_it.o \
./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/system_stm32h7xx.o 

C_DEPS += \
./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/ICMSPI.d \
./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/Kalman2State.d \
./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/MS5607SPI.d \
./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/main.d \
./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/stm32h7xx_hal_msp.d \
./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/stm32h7xx_it.d \
./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/system_stm32h7xx.d 


# Each subdirectory must supply rules for building sources it contributes
.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/%.o .claude/worktrees/elastic-hugle-2aa6b3/Core/Src/%.su .claude/worktrees/elastic-hugle-2aa6b3/Core/Src/%.cyclo: ../.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/%.c .claude/worktrees/elastic-hugle-2aa6b3/Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean--2e-claude-2f-worktrees-2f-elastic-2d-hugle-2d-2aa6b3-2f-Core-2f-Src

clean--2e-claude-2f-worktrees-2f-elastic-2d-hugle-2d-2aa6b3-2f-Core-2f-Src:
	-$(RM) ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/ICMSPI.cyclo ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/ICMSPI.d ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/ICMSPI.o ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/ICMSPI.su ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/Kalman2State.cyclo ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/Kalman2State.d ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/Kalman2State.o ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/Kalman2State.su ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/MS5607SPI.cyclo ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/MS5607SPI.d ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/MS5607SPI.o ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/MS5607SPI.su ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/main.cyclo ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/main.d ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/main.o ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/main.su ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/stm32h7xx_hal_msp.cyclo ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/stm32h7xx_hal_msp.d ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/stm32h7xx_hal_msp.o ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/stm32h7xx_hal_msp.su ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/stm32h7xx_it.cyclo ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/stm32h7xx_it.d ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/stm32h7xx_it.o ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/stm32h7xx_it.su ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/system_stm32h7xx.cyclo ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/system_stm32h7xx.d ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/system_stm32h7xx.o ./.claude/worktrees/elastic-hugle-2aa6b3/Core/Src/system_stm32h7xx.su

.PHONY: clean--2e-claude-2f-worktrees-2f-elastic-2d-hugle-2d-2aa6b3-2f-Core-2f-Src


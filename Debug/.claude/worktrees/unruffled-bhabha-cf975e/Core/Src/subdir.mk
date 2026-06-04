################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/MS5607SPI.c \
../.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/main.c \
../.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/stm32h7xx_hal_msp.c \
../.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/stm32h7xx_it.c \
../.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/system_stm32h7xx.c 

OBJS += \
./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/MS5607SPI.o \
./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/main.o \
./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/stm32h7xx_hal_msp.o \
./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/stm32h7xx_it.o \
./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/system_stm32h7xx.o 

C_DEPS += \
./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/MS5607SPI.d \
./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/main.d \
./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/stm32h7xx_hal_msp.d \
./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/stm32h7xx_it.d \
./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/system_stm32h7xx.d 


# Each subdirectory must supply rules for building sources it contributes
.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/%.o .claude/worktrees/unruffled-bhabha-cf975e/Core/Src/%.su .claude/worktrees/unruffled-bhabha-cf975e/Core/Src/%.cyclo: ../.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/%.c .claude/worktrees/unruffled-bhabha-cf975e/Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean--2e-claude-2f-worktrees-2f-unruffled-2d-bhabha-2d-cf975e-2f-Core-2f-Src

clean--2e-claude-2f-worktrees-2f-unruffled-2d-bhabha-2d-cf975e-2f-Core-2f-Src:
	-$(RM) ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/MS5607SPI.cyclo ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/MS5607SPI.d ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/MS5607SPI.o ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/MS5607SPI.su ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/main.cyclo ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/main.d ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/main.o ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/main.su ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/stm32h7xx_hal_msp.cyclo ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/stm32h7xx_hal_msp.d ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/stm32h7xx_hal_msp.o ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/stm32h7xx_hal_msp.su ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/stm32h7xx_it.cyclo ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/stm32h7xx_it.d ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/stm32h7xx_it.o ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/stm32h7xx_it.su ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/system_stm32h7xx.cyclo ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/system_stm32h7xx.d ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/system_stm32h7xx.o ./.claude/worktrees/unruffled-bhabha-cf975e/Core/Src/system_stm32h7xx.su

.PHONY: clean--2e-claude-2f-worktrees-2f-unruffled-2d-bhabha-2d-cf975e-2f-Core-2f-Src


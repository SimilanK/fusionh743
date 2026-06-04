################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../.claude/worktrees/unruffled-bhabha-cf975e/Middlewares/Third_Party/FatFs/src/option/syscall.c 

OBJS += \
./.claude/worktrees/unruffled-bhabha-cf975e/Middlewares/Third_Party/FatFs/src/option/syscall.o 

C_DEPS += \
./.claude/worktrees/unruffled-bhabha-cf975e/Middlewares/Third_Party/FatFs/src/option/syscall.d 


# Each subdirectory must supply rules for building sources it contributes
.claude/worktrees/unruffled-bhabha-cf975e/Middlewares/Third_Party/FatFs/src/option/%.o .claude/worktrees/unruffled-bhabha-cf975e/Middlewares/Third_Party/FatFs/src/option/%.su .claude/worktrees/unruffled-bhabha-cf975e/Middlewares/Third_Party/FatFs/src/option/%.cyclo: ../.claude/worktrees/unruffled-bhabha-cf975e/Middlewares/Third_Party/FatFs/src/option/%.c .claude/worktrees/unruffled-bhabha-cf975e/Middlewares/Third_Party/FatFs/src/option/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean--2e-claude-2f-worktrees-2f-unruffled-2d-bhabha-2d-cf975e-2f-Middlewares-2f-Third_Party-2f-FatFs-2f-src-2f-option

clean--2e-claude-2f-worktrees-2f-unruffled-2d-bhabha-2d-cf975e-2f-Middlewares-2f-Third_Party-2f-FatFs-2f-src-2f-option:
	-$(RM) ./.claude/worktrees/unruffled-bhabha-cf975e/Middlewares/Third_Party/FatFs/src/option/syscall.cyclo ./.claude/worktrees/unruffled-bhabha-cf975e/Middlewares/Third_Party/FatFs/src/option/syscall.d ./.claude/worktrees/unruffled-bhabha-cf975e/Middlewares/Third_Party/FatFs/src/option/syscall.o ./.claude/worktrees/unruffled-bhabha-cf975e/Middlewares/Third_Party/FatFs/src/option/syscall.su

.PHONY: clean--2e-claude-2f-worktrees-2f-unruffled-2d-bhabha-2d-cf975e-2f-Middlewares-2f-Third_Party-2f-FatFs-2f-src-2f-option


################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../.claude/worktrees/elastic-hugle-2aa6b3/FATFS/App/fatfs.c 

OBJS += \
./.claude/worktrees/elastic-hugle-2aa6b3/FATFS/App/fatfs.o 

C_DEPS += \
./.claude/worktrees/elastic-hugle-2aa6b3/FATFS/App/fatfs.d 


# Each subdirectory must supply rules for building sources it contributes
.claude/worktrees/elastic-hugle-2aa6b3/FATFS/App/%.o .claude/worktrees/elastic-hugle-2aa6b3/FATFS/App/%.su .claude/worktrees/elastic-hugle-2aa6b3/FATFS/App/%.cyclo: ../.claude/worktrees/elastic-hugle-2aa6b3/FATFS/App/%.c .claude/worktrees/elastic-hugle-2aa6b3/FATFS/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean--2e-claude-2f-worktrees-2f-elastic-2d-hugle-2d-2aa6b3-2f-FATFS-2f-App

clean--2e-claude-2f-worktrees-2f-elastic-2d-hugle-2d-2aa6b3-2f-FATFS-2f-App:
	-$(RM) ./.claude/worktrees/elastic-hugle-2aa6b3/FATFS/App/fatfs.cyclo ./.claude/worktrees/elastic-hugle-2aa6b3/FATFS/App/fatfs.d ./.claude/worktrees/elastic-hugle-2aa6b3/FATFS/App/fatfs.o ./.claude/worktrees/elastic-hugle-2aa6b3/FATFS/App/fatfs.su

.PHONY: clean--2e-claude-2f-worktrees-2f-elastic-2d-hugle-2d-2aa6b3-2f-FATFS-2f-App


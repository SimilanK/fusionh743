################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/bsp_driver_sd.c \
../.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/fatfs_platform.c \
../.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/sd_diskio.c 

OBJS += \
./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/bsp_driver_sd.o \
./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/fatfs_platform.o \
./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/sd_diskio.o 

C_DEPS += \
./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/bsp_driver_sd.d \
./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/fatfs_platform.d \
./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/sd_diskio.d 


# Each subdirectory must supply rules for building sources it contributes
.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/%.o .claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/%.su .claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/%.cyclo: ../.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/%.c .claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean--2e-claude-2f-worktrees-2f-unruffled-2d-bhabha-2d-cf975e-2f-FATFS-2f-Target

clean--2e-claude-2f-worktrees-2f-unruffled-2d-bhabha-2d-cf975e-2f-FATFS-2f-Target:
	-$(RM) ./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/bsp_driver_sd.cyclo ./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/bsp_driver_sd.d ./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/bsp_driver_sd.o ./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/bsp_driver_sd.su ./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/fatfs_platform.cyclo ./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/fatfs_platform.d ./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/fatfs_platform.o ./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/fatfs_platform.su ./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/sd_diskio.cyclo ./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/sd_diskio.d ./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/sd_diskio.o ./.claude/worktrees/unruffled-bhabha-cf975e/FATFS/Target/sd_diskio.su

.PHONY: clean--2e-claude-2f-worktrees-2f-unruffled-2d-bhabha-2d-cf975e-2f-FATFS-2f-Target


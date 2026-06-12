################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/diskio.c \
../.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/ff.c \
../.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/ff_gen_drv.c 

OBJS += \
./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/diskio.o \
./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/ff.o \
./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/ff_gen_drv.o 

C_DEPS += \
./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/diskio.d \
./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/ff.d \
./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/ff_gen_drv.d 


# Each subdirectory must supply rules for building sources it contributes
.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/%.o .claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/%.su .claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/%.cyclo: ../.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/%.c .claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean--2e-claude-2f-worktrees-2f-elastic-2d-hugle-2d-2aa6b3-2f-Middlewares-2f-Third_Party-2f-FatFs-2f-src

clean--2e-claude-2f-worktrees-2f-elastic-2d-hugle-2d-2aa6b3-2f-Middlewares-2f-Third_Party-2f-FatFs-2f-src:
	-$(RM) ./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/diskio.cyclo ./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/diskio.d ./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/diskio.o ./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/diskio.su ./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/ff.cyclo ./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/ff.d ./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/ff.o ./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/ff.su ./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/ff_gen_drv.cyclo ./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/ff_gen_drv.d ./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/ff_gen_drv.o ./.claude/worktrees/elastic-hugle-2aa6b3/Middlewares/Third_Party/FatFs/src/ff_gen_drv.su

.PHONY: clean--2e-claude-2f-worktrees-2f-elastic-2d-hugle-2d-2aa6b3-2f-Middlewares-2f-Third_Party-2f-FatFs-2f-src


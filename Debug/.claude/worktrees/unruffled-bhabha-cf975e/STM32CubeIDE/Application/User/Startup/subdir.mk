################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../.claude/worktrees/unruffled-bhabha-cf975e/STM32CubeIDE/Application/User/Startup/startup_stm32h743vitx.s 

OBJS += \
./.claude/worktrees/unruffled-bhabha-cf975e/STM32CubeIDE/Application/User/Startup/startup_stm32h743vitx.o 

S_DEPS += \
./.claude/worktrees/unruffled-bhabha-cf975e/STM32CubeIDE/Application/User/Startup/startup_stm32h743vitx.d 


# Each subdirectory must supply rules for building sources it contributes
.claude/worktrees/unruffled-bhabha-cf975e/STM32CubeIDE/Application/User/Startup/%.o: ../.claude/worktrees/unruffled-bhabha-cf975e/STM32CubeIDE/Application/User/Startup/%.s .claude/worktrees/unruffled-bhabha-cf975e/STM32CubeIDE/Application/User/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m7 -g3 -DDEBUG -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean--2e-claude-2f-worktrees-2f-unruffled-2d-bhabha-2d-cf975e-2f-STM32CubeIDE-2f-Application-2f-User-2f-Startup

clean--2e-claude-2f-worktrees-2f-unruffled-2d-bhabha-2d-cf975e-2f-STM32CubeIDE-2f-Application-2f-User-2f-Startup:
	-$(RM) ./.claude/worktrees/unruffled-bhabha-cf975e/STM32CubeIDE/Application/User/Startup/startup_stm32h743vitx.d ./.claude/worktrees/unruffled-bhabha-cf975e/STM32CubeIDE/Application/User/Startup/startup_stm32h743vitx.o

.PHONY: clean--2e-claude-2f-worktrees-2f-unruffled-2d-bhabha-2d-cf975e-2f-STM32CubeIDE-2f-Application-2f-User-2f-Startup


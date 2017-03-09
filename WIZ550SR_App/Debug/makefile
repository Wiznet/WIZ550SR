################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include system/src/stm32f1-stdperiph/subdir.mk
-include system/src/newlib/subdir.mk
-include system/src/diag/subdir.mk
-include system/src/cortexm/subdir.mk
-include system/src/cmsis/subdir.mk
-include src/ioLibrary/Internet/httpServer/subdir.mk
-include src/ioLibrary/Internet/TFTP/subdir.mk
-include src/ioLibrary/Internet/SNTP/subdir.mk
-include src/ioLibrary/Internet/SNMP/subdir.mk
-include src/ioLibrary/Internet/MQTT/MQTTPacket/src/subdir.mk
-include src/ioLibrary/Internet/MQTT/subdir.mk
-include src/ioLibrary/Internet/FTPServer/subdir.mk
-include src/ioLibrary/Internet/FTPClient/subdir.mk
-include src/ioLibrary/Internet/DNS/subdir.mk
-include src/ioLibrary/Internet/DHCP/subdir.mk
-include src/ioLibrary/Ethernet/W5500/subdir.mk
-include src/ioLibrary/Ethernet/subdir.mk
-include src/ioLibrary/Application/loopback/subdir.mk
-include src/callback/subdir.mk
-include src/S2E/subdir.mk
-include src/PlatformHandler/subdir.mk
-include src/Configuration/subdir.mk
-include src/ATcmd/library/subdir.mk
-include src/ATcmd/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
WIZ550SR_App.bin \

SECONDARY_SIZE += \
WIZ550SR_App.siz \


# All Target
all: WIZ550SR_App.elf secondary-outputs

# Tool invocations
WIZ550SR_App.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM C++ Linker'
	arm-none-eabi-g++ -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -T mem.ld -T libs.ld -T sections.ld -nostartfiles -Xlinker --gc-sections -L"../ldscripts" -Wl,-Map,"WIZ550SR_App.map" --specs=nano.specs -o "WIZ550SR_App.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

WIZ550SR_App.bin: WIZ550SR_App.elf
	@echo 'Invoking: Cross ARM GNU Create Flash Image'
	arm-none-eabi-objcopy -O binary "WIZ550SR_App.elf"  "WIZ550SR_App.bin"
	@echo 'Finished building: $@'
	@echo ' '

WIZ550SR_App.siz: WIZ550SR_App.elf
	@echo 'Invoking: Cross ARM GNU Print Size'
	arm-none-eabi-size --format=berkeley "WIZ550SR_App.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(SECONDARY_FLASH)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS)$(CPP_DEPS) WIZ550SR_App.elf
	-@echo ' '

post-build:
	-arm-none-eabi-objcopy -O ihex "WIZ550SR_App.elf" "WIZ550SR_App.hex"
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets

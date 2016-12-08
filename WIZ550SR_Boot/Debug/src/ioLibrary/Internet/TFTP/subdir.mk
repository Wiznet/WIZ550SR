################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ioLibrary/Internet/TFTP/netutil.c \
../src/ioLibrary/Internet/TFTP/tftp.c 

OBJS += \
./src/ioLibrary/Internet/TFTP/netutil.o \
./src/ioLibrary/Internet/TFTP/tftp.o 

C_DEPS += \
./src/ioLibrary/Internet/TFTP/netutil.d \
./src/ioLibrary/Internet/TFTP/tftp.d 


# Each subdirectory must supply rules for building sources it contributes
src/ioLibrary/Internet/TFTP/%.o: ../src/ioLibrary/Internet/TFTP/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g3 -DDEBUG -DUSE_FULL_ASSERT -DSTM32F10X_HD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=12000000 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -I../src/ioLibrary -I../src/ioLibrary/Ethernet -I../src/ioLibrary/Ethernet/W5500 -I../src/Configuration -I../src/ioLibrary/Internet/TFTP -I../src/PlatformHandler -I../src -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



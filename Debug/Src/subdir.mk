################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/LSM9DS1.c \
../Src/TCA9548A.c \
../Src/glove_status_codes.c \
../Src/hand.c \
../Src/main.c \
../Src/queue.c \
../Src/scheduler.c \
../Src/serial.c \
../Src/sm.c \
../Src/sm_states.c \
../Src/stm32l4xx_hal_msp.c \
../Src/stm32l4xx_it.c \
../Src/syscalls.c \
../Src/system_stm32l4xx.c 

OBJS += \
./Src/LSM9DS1.o \
./Src/TCA9548A.o \
./Src/glove_status_codes.o \
./Src/hand.o \
./Src/main.o \
./Src/queue.o \
./Src/scheduler.o \
./Src/serial.o \
./Src/sm.o \
./Src/sm_states.o \
./Src/stm32l4xx_hal_msp.o \
./Src/stm32l4xx_it.o \
./Src/syscalls.o \
./Src/system_stm32l4xx.o 

C_DEPS += \
./Src/LSM9DS1.d \
./Src/TCA9548A.d \
./Src/glove_status_codes.d \
./Src/hand.d \
./Src/main.d \
./Src/queue.d \
./Src/scheduler.d \
./Src/serial.d \
./Src/sm.d \
./Src/sm_states.d \
./Src/stm32l4xx_hal_msp.d \
./Src/stm32l4xx_it.d \
./Src/syscalls.d \
./Src/system_stm32l4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32L452xx -I"/Users/tejasvi/FydpProto/Inc" -I"/Users/tejasvi/FydpProto/Drivers/STM32L4xx_HAL_Driver/Inc" -I"/Users/tejasvi/FydpProto/Drivers/STM32L4xx_HAL_Driver/Inc/Legacy" -I"/Users/tejasvi/FydpProto/Drivers/CMSIS/Device/ST/STM32L4xx/Include" -I"/Users/tejasvi/FydpProto/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



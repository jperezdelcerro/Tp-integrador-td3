################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/FreeRTOSCommonHooks.c \
../src/heap_3.c \
../src/list.c \
../src/port.c \
../src/queue.c \
../src/tasks.c 

OBJS += \
./src/FreeRTOSCommonHooks.o \
./src/heap_3.o \
./src/list.o \
./src/port.o \
./src/queue.o \
./src/tasks.o 

C_DEPS += \
./src/FreeRTOSCommonHooks.d \
./src/heap_3.d \
./src/list.d \
./src/port.d \
./src/queue.d \
./src/tasks.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_LPCOPEN -D__LPC17XX__ -I"E:\UTN\TDIII\Tp-integrador-td3\Workspace\freertos\inc" -I"E:\UTN\TDIII\Tp-integrador-td3\Workspace\lpc_board_nxp_lpcxpresso_1769\inc" -I"E:\UTN\TDIII\Tp-integrador-td3\Workspace\lpc_chip_175x_6x\inc" -I"E:\UTN\TDIII\Tp-integrador-td3\Workspace\freertos\inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m3 -mthumb -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



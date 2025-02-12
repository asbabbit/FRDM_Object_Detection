################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/Sensors.c \
../source/mtb.c \
../source/semihost_hardfault.c 

C_DEPS += \
./source/Sensors.d \
./source/mtb.d \
./source/semihost_hardfault.d 

OBJS += \
./source/Sensors.o \
./source/mtb.o \
./source/semihost_hardfault.o 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_MKL46Z256VLL4 -DCPU_MKL46Z256VLL4_cm0plus -DSDK_OS_BAREMETAL -DFSL_RTOS_BM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"C:\Users\babbi\Documents\MCUXpressoIDE_11.6.1_8255\workspace\Sensors\board" -I"C:\Users\babbi\Documents\MCUXpressoIDE_11.6.1_8255\workspace\Sensors\source" -I"C:\Users\babbi\Documents\MCUXpressoIDE_11.6.1_8255\workspace\Sensors" -I"C:\Users\babbi\Documents\MCUXpressoIDE_11.6.1_8255\workspace\Sensors\drivers" -I"C:\Users\babbi\Documents\MCUXpressoIDE_11.6.1_8255\workspace\Sensors\CMSIS" -I"C:\Users\babbi\Documents\MCUXpressoIDE_11.6.1_8255\workspace\Sensors\utilities" -I"C:\Users\babbi\Documents\MCUXpressoIDE_11.6.1_8255\workspace\Sensors\startup" -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source

clean-source:
	-$(RM) ./source/Sensors.d ./source/Sensors.o ./source/mtb.d ./source/mtb.o ./source/semihost_hardfault.d ./source/semihost_hardfault.o

.PHONY: clean-source


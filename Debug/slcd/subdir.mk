################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../slcd/GDH-1247WP.c \
../slcd/slcd_engine.c 

C_DEPS += \
./slcd/GDH-1247WP.d \
./slcd/slcd_engine.d 

OBJS += \
./slcd/GDH-1247WP.o \
./slcd/slcd_engine.o 


# Each subdirectory must supply rules for building sources it contributes
slcd/%.o: ../slcd/%.c slcd/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_MCXC444VLH -DCPU_MCXC444VLH_cm0plus -DSDK_OS_BAREMETAL -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -DSERIAL_PORT_TYPE_UART=1 -DSDK_OS_FREE_RTOS -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -DSLCD_PANEL_GDH_1247WP_H -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\drivers" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\CMSIS" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\CMSIS\m-profile" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\utilities" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\utilities\debug_console\config" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\device" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\device\periph2" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\utilities\debug_console" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\component\serial_manager" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\component\lists" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\utilities\str" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\component\uart" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\freertos\freertos-kernel\include" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\freertos\freertos-kernel\portable\GCC\ARM_CM0" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\slcd" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\board" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\freertos\freertos-kernel\template" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\freertos\freertos-kernel\template\ARM_CM0" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\source" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-slcd

clean-slcd:
	-$(RM) ./slcd/GDH-1247WP.d ./slcd/GDH-1247WP.o ./slcd/slcd_engine.d ./slcd/slcd_engine.o

.PHONY: clean-slcd


################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/fsl_clock.c \
../drivers/fsl_common.c \
../drivers/fsl_common_arm.c \
../drivers/fsl_dma.c \
../drivers/fsl_dmamux.c \
../drivers/fsl_gpio.c \
../drivers/fsl_lpuart.c \
../drivers/fsl_lpuart_cmsis.c \
../drivers/fsl_lpuart_dma.c \
../drivers/fsl_slcd.c \
../drivers/fsl_smc.c 

C_DEPS += \
./drivers/fsl_clock.d \
./drivers/fsl_common.d \
./drivers/fsl_common_arm.d \
./drivers/fsl_dma.d \
./drivers/fsl_dmamux.d \
./drivers/fsl_gpio.d \
./drivers/fsl_lpuart.d \
./drivers/fsl_lpuart_cmsis.d \
./drivers/fsl_lpuart_dma.d \
./drivers/fsl_slcd.d \
./drivers/fsl_smc.d 

OBJS += \
./drivers/fsl_clock.o \
./drivers/fsl_common.o \
./drivers/fsl_common_arm.o \
./drivers/fsl_dma.o \
./drivers/fsl_dmamux.o \
./drivers/fsl_gpio.o \
./drivers/fsl_lpuart.o \
./drivers/fsl_lpuart_cmsis.o \
./drivers/fsl_lpuart_dma.o \
./drivers/fsl_slcd.o \
./drivers/fsl_smc.o 


# Each subdirectory must supply rules for building sources it contributes
drivers/%.o: ../drivers/%.c drivers/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_MCXC444VLH -DCPU_MCXC444VLH_cm0plus -DSDK_OS_BAREMETAL -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -DSERIAL_PORT_TYPE_UART=1 -DSDK_OS_FREE_RTOS -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -DSLCD_PANEL_GDH_1247WP_H -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\drivers" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\CMSIS" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\CMSIS\m-profile" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\utilities" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\utilities\debug_console\config" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\device" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\device\periph2" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\utilities\debug_console" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\component\serial_manager" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\component\lists" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\utilities\str" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\component\uart" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\freertos\freertos-kernel\include" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\freertos\freertos-kernel\portable\GCC\ARM_CM0" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\slcd" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\CMSIS_driver\Include" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\board" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\freertos\freertos-kernel\template" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\freertos\freertos-kernel\template\ARM_CM0" -I"C:\Users\johnk\OneDrive\Documents\Software\cg2271\CG2271_Final_Project\source" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-drivers

clean-drivers:
	-$(RM) ./drivers/fsl_clock.d ./drivers/fsl_clock.o ./drivers/fsl_common.d ./drivers/fsl_common.o ./drivers/fsl_common_arm.d ./drivers/fsl_common_arm.o ./drivers/fsl_dma.d ./drivers/fsl_dma.o ./drivers/fsl_dmamux.d ./drivers/fsl_dmamux.o ./drivers/fsl_gpio.d ./drivers/fsl_gpio.o ./drivers/fsl_lpuart.d ./drivers/fsl_lpuart.o ./drivers/fsl_lpuart_cmsis.d ./drivers/fsl_lpuart_cmsis.o ./drivers/fsl_lpuart_dma.d ./drivers/fsl_lpuart_dma.o ./drivers/fsl_slcd.d ./drivers/fsl_slcd.o ./drivers/fsl_smc.d ./drivers/fsl_smc.o

.PHONY: clean-drivers


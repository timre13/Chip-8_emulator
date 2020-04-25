################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Chip-8.cpp \
../DoubleAsker.cpp \
../main.cpp \
../sdl_file_chooser.cpp 

OBJS += \
./Chip-8.o \
./DoubleAsker.o \
./main.o \
./sdl_file_chooser.o 

CPP_DEPS += \
./Chip-8.d \
./DoubleAsker.d \
./main.d \
./sdl_file_chooser.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++17 -I/usr/include/SDL2 -O0 -g3 -Wall -c -fmessage-length=0 -I/usr/include/SDL2 -D_REENTRANT -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



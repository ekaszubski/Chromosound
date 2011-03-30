################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/audio_genome.cpp \
../src/genetic_process.cpp \
../src/test_audio_gene_v1.1.cpp 

OBJS += \
./src/audio_genome.o \
./src/genetic_process.o \
./src/test_audio_gene_v1.1.o 

CPP_DEPS += \
./src/audio_genome.d \
./src/genetic_process.d \
./src/test_audio_gene_v1.1.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



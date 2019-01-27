################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
..\src\WQReader.cpp \
..\src\WQWriter.cpp 

LINK_OBJ += \
.\src\WQReader.cpp.o \
.\src\WQWriter.cpp.o 

CPP_DEPS += \
.\src\WQReader.cpp.d \
.\src\WQWriter.cpp.d 


# Each subdirectory must supply rules for building sources it contributes
src\WQReader.cpp.o: ..\src\WQReader.cpp
	@echo 'Building file: $<'
	@echo 'Starting C++ compile'
	"C:\sloeber\sloeber4.3.1\arduinoPlugin\packages\arduino\tools\avr-gcc\5.4.0-atmel3.6.1-arduino2/bin/avr-g++" -c -g -Os -Wall -Wextra -std=gnu++11 -fpermissive -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -Wno-error=narrowing -MMD -flto -mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=10802 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR     -I"C:\sloeber\sloeber4.3.1\arduinoPlugin\packages\arduino\hardware\avr\1.6.23\variants\standard" -I"C:\sloeber\sloeber4.3.1\arduinoPlugin\libraries\ArduinoQueue\1.0.1" -I"C:\sloeber\sloeber4.3.1\arduinoPlugin\packages\arduino\hardware\avr\1.6.23\libraries\Wire\src" -I"C:\Users\User\git\ardulibrepo\XToolsLib\src" -I"C:\sloeber\sloeber4.3.1\arduinoPlugin\packages\arduino\hardware\avr\1.6.23\cores\arduino" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -D__IN_ECLIPSE__=1 -x c++ "$<"  -o  "$@"
	@echo 'Finished building: $<'
	@echo ' '

src\WQWriter.cpp.o: ..\src\WQWriter.cpp
	@echo 'Building file: $<'
	@echo 'Starting C++ compile'
	"C:\sloeber\sloeber4.3.1\arduinoPlugin\packages\arduino\tools\avr-gcc\5.4.0-atmel3.6.1-arduino2/bin/avr-g++" -c -g -Os -Wall -Wextra -std=gnu++11 -fpermissive -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -Wno-error=narrowing -MMD -flto -mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=10802 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR     -I"C:\sloeber\sloeber4.3.1\arduinoPlugin\packages\arduino\hardware\avr\1.6.23\variants\standard" -I"C:\sloeber\sloeber4.3.1\arduinoPlugin\libraries\ArduinoQueue\1.0.1" -I"C:\sloeber\sloeber4.3.1\arduinoPlugin\packages\arduino\hardware\avr\1.6.23\libraries\Wire\src" -I"C:\Users\User\git\ardulibrepo\XToolsLib\src" -I"C:\sloeber\sloeber4.3.1\arduinoPlugin\packages\arduino\hardware\avr\1.6.23\cores\arduino" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -D__IN_ECLIPSE__=1 -x c++ "$<"  -o  "$@"
	@echo 'Finished building: $<'
	@echo ' '



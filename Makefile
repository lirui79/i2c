all: testi2c libi2cdev.so test_i2c testbat adjustByRtc


ifeq ($(CC), )
CC := gcc
endif
$(info "CC is:$(CC)")

ifeq ($(CXX), )
CXX := g++
endif
$(info "CXX is:$(CXX)")

#CROSS_COMPILE   = arm-linux-gnueabihf-
#CROSS_COMPILE   = arm-none-eabi-
GXX		= $(CXX)
#$(CROSS_COMPILE)g++
#GXX             = g++ -std=c++11
#GCC		= $(CROSS_COMPILE)gcc -std=c99, -std=gnu99, -std=c11 
GCC		= $(CC) -std=c11 
#-std=c99, -std=gnu99, -std=c11 
#GCC             = gcc

testi2c: testi2c.c i2cdev.c
	$(GCC) testi2c.c i2cdev.c -L. -o testi2c

libi2cdev.so: i2cdev.c
	$(GCC) i2cdev.c  -fPIC -shared -L./ -o libi2cdev.so

testbat: testBatteryMng.cpp BatteryMng.cpp
	$(GXX) testBatteryMng.cpp BatteryMng.cpp -L. -o testbat

test_i2c: testi2c.c libi2cdev.so
	$(GCC) testi2c.c -L. -li2cdev -o test_i2c

adjustByRtc: adjustSystemTimeByRTC.c i2cdev.c
	$(GCC) adjustSystemTimeByRTC.c i2cdev.c -o adjustByRtc


clean:
	rm  *.so testi2c test_i2c testbat

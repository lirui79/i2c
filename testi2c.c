/*
This software uses a BSD license.

Copyright (c) 2010, Sean Cross / chumby industries
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

* Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the
   distribution. 
* Neither the name of Sean Cross / chumby industries nor the names
   of its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#include "i2cdev.h"

//#define I2C_FILE_NAME "/dev/i2c-1"
#define USAGE_MESSAGE \
    "Usage:\n" \
    "  %s mode devName [addr] [register]   " \
        "to read value from [register]\n" \
    "  %s mode devName [addr] [register] [value]   " \
        "to write a value [value] to register [register]\n" \
    ""

int i2c_test_byte(const char *devName, int mode, int addr, int reg, int val);
int i2c_test_file(const char *devName, int mode, int addr, int reg, int val);
int i2c_test_data(const char *devName, int mode, int addr, int reg, const char* val);


// ./testi2c 0 /dev/i2c-1 0x50 0
// ./testi2c 1 /dev/i2c-1 0x50 0 "scb3100000001"
// ./testi2c 2 /dev/i2c-1 0x50 0x10
// ./testi2c 3 /dev/i2c-1 0x50 0x10 
int main(int argc, char **argv)
{
    const char *devName = NULL;
    int mode = 0, addr = 0, reg = 0, val = 0;

    if (argc < 4) {
       fprintf(stderr, USAGE_MESSAGE, argv[0], argv[0]);
       printf("read  %s 0 /dev/i2c-1 0x56 0\n", argv[0]);
       printf("write %s 1 /dev/i2c-1 0x56 0 scb3100000001\n", argv[0]);
       printf("read  %s 2 /dev/i2c-1 0x50 0x10\n", argv[0]);
       printf("write %s 3 /dev/i2c-1 0x50 0x10 100\n", argv[0]);
       printf("read  %s 4 /dev/i2c-1 0x50 0x10\n", argv[0]);
       printf("write %s 5 /dev/i2c-1 0x50 0x10 100\n", argv[0]);
       return -1;
    }
    
    mode = strtol(argv[1], NULL, 0);
    devName = argv[2];
    addr = strtol(argv[3], NULL, 0);
    reg = strtol(argv[4], NULL, 0);
    if (argc >= 6) {
        if (mode != 1)
           val = strtol(argv[5], NULL, 0);
    }

    if (mode < 2) {
        printf("i2c_test_data begin\n");
        if(mode == 0)
            i2c_test_data(devName, mode, addr, reg, "empty");
        else if(mode = 1)
            i2c_test_data(devName, mode, addr, reg, argv[5]);
    } else if (mode < 4) {
        mode = mode - 2;
        printf("i2c_test_byte begin\n");
        i2c_test_byte(devName, mode, addr, reg, val);
    } else {
        mode = mode - 4;
        printf("i2c_test_file begin\n");
        i2c_test_file(devName, mode, addr, reg, val);
    }


    return 0;
}


int i2c_test_data(const char *devName, int mode, int addr, int reg, const char* val)
{
    int  fd = i2c_open(devName, addr);
    // Open a connection to the I2C userspace control file.
    if (fd < 0) {
        perror("Unable to open i2c control file");
        return -2;
    }

    //const char temp[] = "scb3100000001";
    unsigned char buffer[16] = {0};
    unsigned int size = 16;
    sprintf(buffer, "%s", val);
    if(mode == 0) {
        memset(buffer, 0, 16);
        if(i2c_read_data(fd, addr, reg, buffer, size)) {
            printf("Unable to read register!\n");
        }
        else {
            const char *temp = (const char*) buffer;
            printf("--read Register:%d, val: %s, sizeof(tmp):%d, strlen(tmp):%u\n",
                 reg, temp, sizeof(temp), strlen(temp));
            for (int i = 0; i < size; ++i) {
               printf("%02x ", buffer[i]);
            }
            printf("\n");
        }
    }
   
   if(mode == 1) {
        size = 14;
        const char *temp = (const char*) buffer;
        if(i2c_write_data(fd, addr, reg, buffer, size)) {
            printf("Unable to read register!\n");
        }
        else {
            printf("write Register:%d %s\n",reg, temp);
            for (int i = 0; i < size; ++i) {
               printf("%02x ", buffer[i]);
            }
            printf("\n");
        }
    }

    close(fd);

    return 0;
}

int i2c_test_file(const char *devName, int mode, int addr, int reg, int val)
{
    int  fd = i2c_open(devName, addr);
    // Open a connection to the I2C userspace control file.
    if (fd < 0) {
        perror("Unable to open i2c control file");
        return -2;
    }

    unsigned char buffer[16] = {0};
    unsigned int size = 16;
    if(mode == 0) {
        if(i2c_read(fd, reg, buffer, size)) {
            printf("Unable to read register!\n");
        }
        else {
            printf("read Register:%d \n",reg);
            for (int i = 0; i < size; ++i) {
               printf("%02x ", buffer[i]);
            }
            printf("\n");
        }
    }
   
   if(mode == 1) {
        for (int i = 0; i < size; ++i) {
            buffer[i] = val + i;
        }
        if(i2c_write(fd, reg, buffer, size)) {
            printf("Unable to read register!\n");
        }
        else {
            printf("write Register:%d \n",reg);
            for (int i = 0; i < size; ++i) {
               printf("%02x ", buffer[i]);
            }
            printf("\n");
        }
    }

    close(fd);

    return 0;
}


int i2c_test_byte(const char *devName, int mode, int addr, int reg, int val)
{
    int  fd = i2c_open(devName, addr);
    // Open a connection to the I2C userspace control file.
    if (fd < 0) {
        perror("Unable to open i2c control file");
        return -2;
    }

    if(mode == 0) {
        if(i2c_read_register(fd, addr, reg, &val)) {
            printf("Unable to read register!\n");
        }
        else {
            printf("Register %d: %d (%x)\n", reg, (int)val, (int)val);
        }
    }
   
   if(mode == 1) {
        if(i2c_write_register(fd, addr, reg, val)) {
            printf("Unable to write register!\n");
        }
        else {
            printf("write register %x: %d (%x)\n", reg, val, val);
        }
    }

    close(fd);

    return 0;
}

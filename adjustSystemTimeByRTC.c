
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#include "i2cdev.h"

// 该函数移植自 submodule/gps/GpsMng.cpp
int32_t adjustSystemTimeByRTC()
{
    int ret = 0;

    // RTS设备 BL5372的，设备名称和地址
    char devName[] = "/dev/i2c-0";
    unsigned int addr = 0x32;
    unsigned int reg = 0xF0; // 寄存器地址,参照BL5372手册，寄存器地址0-F,在使用时使用1H，此处写成10H

    // ======== 读取时制 12时制 24时制
    unsigned char allByte[7] = {'\0'};
    reg = 0xF0; // 设置计时周期是24时制，
    /*ret = i2cReadAll(devname, addr, reg, allByte, 1);
    if (0 != ret)
    {
        printf("reg:%02X ret:%d i2cReadAll time_config NG\n", (unsigned char)reg, ret);
        return -1;
    }*/
    int fd = i2c_open(devName, addr);
    if (fd < 0) {
        perror("Unable to open i2c control file");
        return -2;
    }
	if(i2c_read_data(fd, addr, reg, allByte, 1)) {
		printf("Unable to read register 0xF0!\n");
		close(fd);
		return -3;
	}	
	
    printf("reg:%02X time_config:%02X i2cReadAll OK\n", (unsigned char)reg, allByte[0]);
    if((allByte[0] & 0x20) == 0) // 判断是否 24 小时制
    //if (0X20 != allByte[0]) // 判断是不是24小时时制
    {
        printf("reg=%02X time_config=%02X not 24H\n", (unsigned char)reg, allByte[0]);
        return -1;
    }
	close(fd);
	fd = -1;
 
    // ===== 读取年月日时分秒
    reg = 0x00; // 寄存器地址 秒 分 时 周 天 月 年
    /*ret = i2cReadAll(devname, addr, reg, allByte, 7);
    if (0 != ret)
    {
        printf("reg:%02X ret:%d i2cReadAll current time NG\n", (unsigned char)reg, ret);
        return -1;
    }
	*/
    fd = i2c_open(devName, addr);
    if (fd < 0) {
        perror("Unable to open i2c control file");
        return -2;
    }
	if(i2c_read_data(fd, addr, reg, allByte, 7)) {
		printf("Unable to read register 0x00!\n");
		close(fd);
		return -3;
	}

    char cur_time[32] = {'\0'};
    char tmpStr[8];
    snprintf(tmpStr, sizeof(tmpStr), "20%d%d-", (allByte[6] & 0XF0) >> 4, allByte[6] & 0X0F);
    strcat(cur_time, tmpStr);
    snprintf(tmpStr, sizeof(tmpStr), "%d%d-", (allByte[5] & 0XF0) >> 4, allByte[5] & 0X0F);
    strcat(cur_time, tmpStr);
    snprintf(tmpStr, sizeof(tmpStr), "%d%d ", (allByte[4] & 0XF0) >> 4, allByte[4] & 0X0F);
    strcat(cur_time, tmpStr);
    snprintf(tmpStr, sizeof(tmpStr), "%d%d:", (allByte[2] & 0XF0) >> 4, allByte[2] & 0X0F);
    strcat(cur_time, tmpStr);
    snprintf(tmpStr, sizeof(tmpStr), "%d%d:", (allByte[1] & 0XF0) >> 4, allByte[1] & 0X0F);
    strcat(cur_time, tmpStr);
    snprintf(tmpStr, sizeof(tmpStr), "%d%d", (allByte[0] & 0XF0) >> 4, allByte[0] & 0X0F);
    strcat(cur_time, tmpStr);
    printf("reg:%02X current time:%s OK\n", (unsigned char)reg, cur_time);

    char cmdTmp[64];
    sprintf(cmdTmp, "date -s \"%s\"", cur_time);
    // date -s "2022-06-17 19:33:05"
    printf("set time:%s into system\n", cmdTmp);
    system(cmdTmp);

    return 0;
}

int32_t main()
{
	return adjustSystemTimeByRTC();
}

/*
 * Copyright (c) 2022.zhongkeruixin Technologies.
 * BatteryMng.cpp
 *
 *  Created on: 2022年4月14日
 *      Author: wulifu
 */
#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include "BatteryMng.h"

BatteryMng::BatteryMng()
{
    // TODO Auto-generated constructor stub
}

BatteryMng::~BatteryMng()
{
    // TODO Auto-generated destructor stub
}

//######################################################################
//######################  public function ##############################
//######################################################################
// 初始化电池模块
int BatteryMng::init()
{
    LOGF_START();
    char devName[] = "/dev/i2c-1";
    unsigned int addr = 0x50, reg = 0x00;
    unsigned char buffer[16] = {0};
    unsigned int size = 16, index = 0;
    char temp[64] = {0};
    int fd = i2c_open(devName, addr), code = 0;
    if( fd < 0) {
        return fd;
    }

    code = i2c_read_data(fd, addr, reg, buffer, size);
    if (code < 0) {
        i2c_close(fd);
        return code;
    }
    for (int i = 0; i < size; ++i)
        index += sprintf(temp + index,"%02x",buffer[i]);

    code = i2c_close(fd);

    code = fwritelock("/etc/device_id", temp, index + 1);

    LOGF_END();
    return 0;
}

/* @description  : 取得剩余电量的  */
int BatteryMng::getPercentage(float &val, int &time_m)
{
    LOGF_START();
    char devName[] = "/dev/i2c-1";
    unsigned int addr = 0x50, reg = 0x00;
    unsigned char buffer[16] = {0};
    unsigned int size = 16;
    int fd = i2c_open(devName, addr), code = 0;
    if( fd < 0) {
        return fd;
    }

    code = i2c_read_data(fd, addr, reg, buffer, size);
    if (code < 0) {
        i2c_close(fd);
        return code;
    }
    val = buffer[0];
    time_m = buffer[1];
    code = i2c_close(fd);

    LOGF_END();
    return 0;
}

/*
 * @description  : 从eeprom中取得箱子ID
 * @param(out)   : boxID ->  箱子ID
 * @return:      0->正常结束    other->处理异常
 */
int BatteryMng::getBoxID(std::string &boxID) {
    LOGF_START();
    char buffer[64] = {0};
    int size = 64;
    freadlock("/etc/device_id", buffer, size);
    boxID = buffer;
    LOGF_END();
    return 0;
}

//######################################################################
//######################  private function #############################
//######################################################################

int BatteryMng::i2c_open(const char* devname, unsigned int addr) {
    int fd = open(devname, O_RDWR);
    if (fd < 0) {
        LOGF_ERR("Unable open %s device\n",devname);
        return -1;
    }

    if(ioctl(fd,I2C_SLAVE,addr) < 0) {
        LOGF_ERR("Unable ioctl %s device address:%d\n",devname, addr);
        close(fd);
        return -2;
    }
    return fd;
}

int BatteryMng::i2c_write_data(int fd, unsigned int addr, unsigned int reg, unsigned char *value, unsigned int size) {
    unsigned char outbuf[20] = {0};
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];
    int code = 0;
    if (size > 16) {
        LOGF_ERR("data size:%d > 16\n",size);
        return -1;
    }
    messages[0].addr  = addr;
    messages[0].flags = 0;//write
    messages[0].len   = size + 1;
    messages[0].buf   = outbuf;

    /* The first byte indicates which register we'll write */
    outbuf[0] = reg;

    /*
     * The second byte indicates the value to write.  Note that for many
     * devices, we can write multiple, sequential registers at once by
     * simply making outbuf bigger.
     */    //outbuf[1] = value;
    memcpy(outbuf + 1, value, size);

    /* Transfer the i2c packets to the kernel and verify it worked */
    packets.msgs  = messages;
    packets.nmsgs = 1;
    code = ioctl(fd, I2C_RDWR, &packets);
    if( code < 0) {
        LOGF_ERR("Unable to send data  %d \n", code);
        return -2;
    }

    return 0;
}

int BatteryMng::i2c_read_data(int fd, unsigned int addr, unsigned int reg, unsigned char *value, unsigned int size) {
    unsigned char inbuf[20] = {0}, outbuf[1] = {0};
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
    int code = 0;
    if (size > 16) {
        LOGF_ERR("data size:%d > 16\n",size);
        return -1;
    }
    /*
     * In order to read a register, we first do a "dummy write" by writing
     * 0 bytes to the register we want to read from.  This is similar to
     * the packet in set_i2c_register, except it's 1 byte rather than 2.
     */
    outbuf[0] = reg;
    messages[0].addr  = addr;
    messages[0].flags = 0;//write
    messages[0].len   = 1;
    messages[0].buf   = outbuf;

    /* The data will get returned in this structure */
    messages[1].addr  = addr;
    messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
    messages[1].len   = size;
    messages[1].buf   = inbuf;

    /* Send the request to the kernel and get the result back */
    packets.msgs      = messages;
    packets.nmsgs     = 2;
    code = ioctl(fd, I2C_RDWR, &packets);
    if( code < 0) {
        LOGF_ERR("Unable to send data  %d \n", code);
        return -2;
    }

    memcpy(value, inbuf, size);
    return 0;
}

int BatteryMng::i2c_close(int fd) {
    if (fd >= 0) {
        close(fd);
    }
    return 0;
}


int BatteryMng::freadlock(const char *tempfile, char *buffer, int size) {
    struct flock lock;
    int fd = open(tempfile,O_RDONLY), ret = 0;
    if(fd < 0) {
       LOGF_ERR("Unable open %s file\n",tempfile);
       return -1;
    }

    lock.l_type=F_RDLCK;
    lock.l_whence=SEEK_SET;
    lock.l_start=0;
    lock.l_len=64;

    ret=fcntl(fd,F_SETLKW,&lock);
    //使用资源
    ret=read(fd,buffer,size);
    if(ret <= 0) {
       LOGF_ERR("Unable read %s file\n",tempfile);
    }

    //解锁
    lock.l_type=F_UNLCK;
    lock.l_whence=SEEK_SET;
    lock.l_start=0;
    lock.l_len=64;
    fcntl(fd,F_SETLKW,&lock);
    close(fd);
    return ret;
}

int BatteryMng::fwritelock(const char *tempfile, const char *buffer, int size) {
    struct flock lock;
    int fd = open(tempfile, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH), ret = 0;
    if(fd < 0) {
        LOGF_ERR("open file:%s-%d", tempfile, fd);
        return -1;
    }
    //设置读锁
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 64;

    //上锁
    ret = fcntl(fd, F_SETLKW, &lock);
    //使用资源
    ret = write(fd, buffer, size);
    if(ret < 0) {
        LOGF_ERR("write file:%s-%d", tempfile, ret);
    }
    //解锁
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 64;
    fcntl(fd, F_SETLKW, &lock);
    close(fd);
    return ret;
}
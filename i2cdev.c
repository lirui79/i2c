#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#include "i2cdev.h"


int i2c_open(const char* devname, unsigned int addr) {
	int fd = open(devname, O_RDWR);
	if (fd < 0) {
        printf("Unable open %s device\n",devname);
		return -1;
	}

	if(ioctl(fd,I2C_SLAVE,addr) < 0) {
        printf("Unable ioctl %s device address:%d\n",devname, addr);
        close(fd);
		return -2;
	}
    return fd;
}


int i2c_write_register(int fd, unsigned int addr, unsigned int reg, unsigned int value) {
    unsigned char outbuf[2];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];
    int code = 0;
    messages[0].addr  = addr;
    messages[0].flags = 0;//write
    messages[0].len   = sizeof(outbuf);
    messages[0].buf   = outbuf;

    /* The first byte indicates which register we'll write */
    outbuf[0] = reg;

    /*
     * The second byte indicates the value to write.  Note that for many
     * devices, we can write multiple, sequential registers at once by
     * simply making outbuf bigger.
     */
    outbuf[1] = value;

    /* Transfer the i2c packets to the kernel and verify it worked */
    packets.msgs  = messages;
    packets.nmsgs = 1;
    code = ioctl(fd, I2C_RDWR, &packets);
    if( code < 0) {
        printf("Unable to send data  %d \n", code);
        return -1;
    }

    return 0;
}

int i2c_read_register(int fd, unsigned int addr, unsigned int reg, unsigned int *value) {
	unsigned char inbuf, outbuf;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
    int code = 0;

    /*
     * In order to read a register, we first do a "dummy write" by writing
     * 0 bytes to the register we want to read from.  This is similar to
     * the packet in set_i2c_register, except it's 1 byte rather than 2.
     */
    outbuf = reg;
    messages[0].addr  = addr;
    messages[0].flags = 0;//write
    messages[0].len   = sizeof(outbuf);
    messages[0].buf   = &outbuf;

    /* The data will get returned in this structure */
    messages[1].addr  = addr;
    messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
    messages[1].len   = sizeof(inbuf);
    messages[1].buf   = &inbuf;

    /* Send the request to the kernel and get the result back */
    packets.msgs      = messages;
    packets.nmsgs     = 2;
    code = ioctl(fd, I2C_RDWR, &packets);
    if( code < 0) {
        printf("Unable to send data  %d \n", code);
        return -1;
    }

    *value = inbuf;
    return 0;
}

#if 0 // 一次性将 value 写入指定寄存器
int i2c_write_data(int fd, unsigned int addr, unsigned int reg, unsigned char *value, unsigned int size) {
    unsigned char outbuf[20] = {0};
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];
    int code = 0;
    if (size > 16) {
        printf("data size:%d > 16\n",size);
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
        printf("Unable to send data  %d \n", code);
        return -1;
    }

    return 0;
}
#else //  逐个字节写入寄存器
#define BOX_ID_MAX_LEN 13
int i2c_write_data(int fd, unsigned int addr, unsigned int reg, unsigned char *value, unsigned int size) {
    int code = 0;
    if (size > 16) {
        printf("data size:%d > 16\n",size);
        return -1;
    }
    unsigned char outbuf[3] = {0};
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];
    messages[0].addr  = addr;
    messages[0].flags = 0;//write
    messages[0].len   = 2;
    messages[0].buf   = outbuf;

    unsigned int i = 0;
    for(i=0; i<BOX_ID_MAX_LEN; i++){
        /* The first byte indicates which register we'll write */
        outbuf[0] = reg+i;
        /*
         * The second byte indicates the value to write.  Note that for many
         * devices, we can write multiple, sequential registers at once by
         * simply making outbuf bigger.
         */    
        outbuf[1] = value[i];
        //printf("i:%d, target reg:%02x, val:%c\n", i, outbuf[0], outbuf[1]);
        //memcpy(outbuf + 1, value, size);

        /* Transfer the i2c packets to the kernel and verify it worked */
        packets.msgs  = messages;
        packets.nmsgs = 1;
        code = ioctl(fd, I2C_RDWR, &packets);
        if( code < 0) {
            printf("Unable to send data  %d \n", code);
            return -1;
        }
		// 逐字节写数据时，需要sleep一会，否则写失败
        usleep(50*1000);
    }
    return 0;
}

#endif

int i2c_read_data(int fd, unsigned int addr, unsigned int reg, unsigned char *value, unsigned int size) {
    unsigned char inbuf[20] = {0}, outbuf[1] = {0};
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
    int code = 0;
    if (size > 16) {
        printf("data size:%d > 16\n",size);
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
        printf("Unable to send data  %d \n", code);
        return -1;
    }

    memcpy(value, inbuf, size);
    return 0;
}


int i2c_write(int fd, unsigned int reg, unsigned char *value, unsigned int size) {
    unsigned char buffer[64] = {0};    
    if (size > 16) {
        printf("data size:%d > 16\n",size);
        return -1;
    }
    buffer[0]=reg;
    if ((value != NULL) && (size > 0))
       memcpy(buffer + 1, value, size);
    write(fd, buffer, size + 1);
	return 0;
}

int i2c_read(int fd, unsigned int reg, unsigned char *value, unsigned int size) {
    if (size > 16) {
        printf("data size:%d > 16\n",size);
        return -1;
    }
	i2c_write(fd, reg, NULL, 0);
    read(fd, value, size);
    return 0;
}

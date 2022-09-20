#ifndef __I2C_DEVICE_INTERFACE_H___
#define __I2C_DEVICE_INTERFACE_H___


#ifdef __cplusplus
extern "C"{
#endif

int i2c_open(const char* devname, unsigned int addr);

//
int i2c_write_register(int fd, unsigned int addr, unsigned int reg, unsigned int value);

int i2c_read_register(int fd, unsigned int addr, unsigned int reg, unsigned int *value);

//
int i2c_write_data(int fd, unsigned int addr, unsigned int reg, unsigned char *value, unsigned int size);

int i2c_read_data(int fd, unsigned int addr, unsigned int reg, unsigned char *value, unsigned int size);

//
int i2c_write(int fd, unsigned int reg, unsigned char *value, unsigned int size);

int i2c_read(int fd, unsigned int reg, unsigned char *value, unsigned int size);




#ifdef __cplusplus
}
#endif

#endif
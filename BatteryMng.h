/*
 * Copyright (c) 2022.zhongkeruixin Technologies.
 * BatteryMng.h
 *
 *  Created on: 2022年4月14日
 *      Author: wulifu 
 */

#ifndef SMART_BOX_V2_SUBMODULE_BATTERY_BATTERYMNG_H_
#define SMART_BOX_V2_SUBMODULE_BATTERY_BATTERYMNG_H_

#include <string>
#include "comLog.h"

class BatteryMng
{
protected:
    int fwritelock(const char *tempfile, const char *buffer, int size);

    int freadlock(const char *tempfile, char *buffer, int size);

    int i2c_open(const char* devname, unsigned int addr);
    //
    int i2c_write_data(int fd, unsigned int addr, unsigned int reg, unsigned char *value, unsigned int size);

    int i2c_read_data(int fd, unsigned int addr, unsigned int reg, unsigned char *value, unsigned int size);

    int i2c_close(int fd);
public:
    BatteryMng();
    virtual ~BatteryMng();

    /*
     * @description  : 初始化电池模块
     * @param        : 无
     * @return:      0->正常结束    other->处理异常
     */
    int init();

    /*
     * @description  : 取得剩余电量的
     * @param(out)   : val ->  剩余电量的百分比，例如剩余百分之80.88%,返回值为80.88
     * @param(out)   : time -> 可持续分钟,例如 可持续2小时3分钟,则返回123。
     * @return:      0->正常结束    other->处理异常
     */
    int getPercentage(float &val, int &time_m);

     /*
     * @description  : 从eeprom中取得箱子ID
     * @param(out)   : boxID ->  箱子ID
     * @return:      0->正常结束    other->处理异常
     */
    int getBoxID(std::string &boxID);
};

#endif /* SMART_BOX_V2_SUBMODULE_BATTERY_BATTERYMNG_H_ */

/*
EEPROM读写程序AT24C08
创建日期：2018年10月29日
作者：孙浩
更新日期：2018年10月31日
作者：孙浩
EEPROM写入和读取长度增加大于16个字节
更新日期：2018年10月31日
作者：孙浩
EEPROM写入和读取起始字节必必须是16的倍数

E2prom_Init()
用于EEPROM的初始化，主要包括GPIO初始化和IIC初始化，在初始化模块中调用
SCL_IO=14               
SDA_IO=18      

int E2prom_Write(uint8_t addr,uint8_t*data_write,int len);
用于写入EEPROM，参数：写入数据地址、指针和写入数据长度，操作扇区为0号扇区
地址范围0x00-0xff
返回值
ESP_OK 写入成功
其他 写入失败

int E2prom_Read(uint8_t addr,uint8_t*data_read,int len);
用于在0号扇区读出数据，参数：读出数据数据地址、指针和读出长度
地址范围0x00-0xff
返回值
ESP_OK 读取成功
其他 读取失败

*/

#ifndef _E2PROM_H_
#define _E2PROM_H_

#include "freertos/FreeRTOS.h"

  /*
  EEPROM PAGE0 
    0x00 Device_id       (16byte)-------注册获得，鉴权使用
    0x10 Serial_No       (16byte)-------烧录获得，鉴权使用
    0x20 RegisterCode    (16byte)-------烧录获得
    0x30 Protuct_id      (16byte)-------烧录获得，鉴权使用
  EEPROM PAGE1
    0X00 none
  */
#define DEVICEID_ADDR       0x00
#define SERIALNUM_ADDR      0x10
#define PRODUCTID_ADDR      0x30
#define REGISTERCODE_ADDR   0x20

#define DEVICEID_LEN        16
#define SERIALNUM_LEN       16
#define PRODUCTID_LEN       16
#define REGISTERCODE_LEN    16

char SerialNum[SERIALNUM_LEN+1];
char ProductId[PRODUCTID_LEN+1];
char RegisterCode[REGISTERCODE_LEN+1];
char Device_id[REGISTERCODE_LEN+1];


extern void E2prom_Init(void);
extern int E2prom_Write(uint8_t addr,uint8_t*data_write,int len);
extern int E2prom_Read(uint8_t addr,uint8_t*data_read,int len);
extern int E2prom_BluWrite(uint8_t addr,uint8_t*data_write,int len);
extern int E2prom_BluRead(uint8_t addr,uint8_t*data_read,int len);
#endif


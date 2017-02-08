#ifndef __W25X16_H
#define __W25X16_H

#include "cc430x513x.h"

#define CS_1  P1OUT |=  BIT7
#define CS_0  P1OUT &=~ BIT7
/* Private typedef -----------------------------------------------------------*/

//#define SPI_FLASH_PageSize      4096
#define SPI_FLASH_PageSize      256
#define SPI_FLASH_PerWritePageSize      256

/* Private define ------------------------------------------------------------*/

#define Write_Enable   0x06   //在每一个写或者擦除操作前必须执行一次 
#define Write_Disable  0x04   //关闭写使能
#define Read_Data      0x03   //读数据
#define Read_StatusReg  0x05 
#define Write_StatusReg 0x01 
#define Register_1     0x05   //读状态寄存器 1
#define Page_Write     0x02   //写一个页的数据  256 byte
#define Sector_Erase   0x20   //段擦除  4KB ，最大需要400ms操作时间
#define Chip_Erase     0xC7   //片擦除 ，最大需要40s
#define Power_Down     0xB9   //掉电模式（低功耗，典型值 1.5 uA）,延时3us后生效
#define Power_Up       0xAB   //从掉电模式中恢复到可操作模式
#define ManufactDeviceID 0x90 //制造商ID
#define JedecDeviceID  0x9F 
#define FastReadData 0x0B 
#define FastReadDual 0x3B 
#define BlockErase  0xD8 
#define DeviceID    0xAB 

#define WIP_Flag                  0x01  /* Write In Progress (WIP) flag */

#define Dummy_Byte                0xFF

void SPI_FLASH_Init(void);
void SPI_Send_Data( unsigned char data);      //SPI发送一个字节数据
unsigned char SPI_Read_Data( void);           //SPI读取一个字节数据
void ExtenalFlash_Write_Enable(void );  // 写使能
unsigned char ExtenalFlash_Read_Register(void );  // 读使能
void ExtenalFlash_Power_Down(void ) ; // 进休眠模式
void ExtenalFlash_Power_Up(void ) ; // 退出休眠模式
void ExtenalFlash_Sector_Erase( long addr);  //flash 擦除操作
void ExtenalFlash_Write(unsigned char *p, int count ,long addr);  //flash 写操作
void ExtenalFlash_Read(unsigned char *p, long Adress , int count) ; //flash 读操作
long ExtenalFlash_ReadID(void);  //读制造商ID
unsigned char SPI_FLASH_ReadDeviceID(void);
void SPI_FLASH_BulkErase(void);
void SPI_FLASH_WaitForWriteEnd(void);


#endif /* __W25X16_H */


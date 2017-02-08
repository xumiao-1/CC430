#include "Si705x.h"
#include "i2c.h"
#include "delay.h"

void Si705x_Init(void)
{
  //P2SEL &= ~ (BIT1+BIT2);
  SCL_OUT;
  SDA_OUT;
  //SDA_LOW;
}

unsigned int getTemperature(void)  //Temperature = 175.52*tmp/65536-46.85 Unit: °„C
{
  unsigned int tmp;

  I2C_START();
  I2C_SendByte(SlaveAddr);
  while (I2C_Check_ACK() == FALSE);
  I2C_SendByte(0xF3);
  while (I2C_Check_ACK() == FALSE);
  I2C_STOP();
  delay_us(9000);
  I2C_START();
  I2C_SendByte(SlaveAddr+1);
  while (I2C_Check_ACK() == FALSE);
  tmp = I2C_RecvByte();
  I2C_SendAck();
  tmp <<= 8;
  tmp |= I2C_RecvByte();
  I2C_SendNoAck();
  I2C_STOP();
  return tmp;
  
}
void WriteUserRegister(unsigned char data)
{
  I2C_START();
  I2C_SendByte(SlaveAddr);
  while (I2C_Check_ACK() == FALSE);
  //while (I2C_Check_ACK() == FALSE);
  I2C_SendByte(WriteRegister1);
  while (I2C_Check_ACK() == FALSE);
  I2C_SendByte(data);
  I2C_SendAck();
  //while (I2C_Check_ACK() == FALSE);
  I2C_STOP();
}

unsigned char ReadUserRegister(void)
{
  unsigned char tmp;
  I2C_START();
  I2C_SendByte(SlaveAddr);
  while (I2C_Check_ACK() == FALSE);
  //while (I2C_Check_ACK() == FALSE);
  I2C_SendByte(ReadRegister1);
  while (I2C_Check_ACK() == FALSE);
  I2C_START();
  I2C_SendByte(SlaveAddr+1);
  while (I2C_Check_ACK() == FALSE);
  tmp = I2C_RecvByte();
  I2C_SendNoAck();
  I2C_STOP();
  return tmp;
}


unsigned char ReadFrimwareRev(void)
{
  unsigned char tmp;

  I2C_START();
  I2C_SendByte(SlaveAddr);
  while (I2C_Check_ACK() == FALSE);
  I2C_SendByte((ReadFirmRev>>8)&0xFF);
  while (I2C_Check_ACK() == FALSE);
  I2C_SendByte(ReadFirmRev&0xFF);
  while (I2C_Check_ACK() == FALSE);
  I2C_START();
  I2C_SendByte(SlaveAddr+1);
  while (I2C_Check_ACK() == FALSE);
  tmp = I2C_RecvByte();
  //while (I2C_Check_ACK() == FALSE);
  while (I2C_Check_ACK() == TRUE);
  I2C_STOP();
  
  return tmp;
  
}

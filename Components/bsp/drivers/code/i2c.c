#include "i2c.h"
#include "delay.h"

void I2C_START(void)
{
  SDA_OUT;
  delay_us(20);
  SDA_HIGH;    
  delay_us(3);
  SCL_HIGH;
  delay_us(4);
  SDA_LOW;
  delay_us(5);
  SCL_LOW;
  delay_us(10);
}

//void I2C_RESTART(void)
//{
//  SDA_OUT;
//  delay_us(20);
//  SDA_HIGH;    
//  delay_us(3);
//  SCL_HIGH;
//  delay_us(4);
//  SDA_LOW;
//  delay_us(5);
//  SCL_LOW;
//  delay_us(10);
//}

void I2C_STOP(void)
{
  SDA_OUT;
  SDA_LOW;
  delay_us(4);
  SCL_HIGH;    
  delay_us(4);
  SDA_LOW;
  delay_us(4);
  SDA_HIGH;
}

void I2C_SendAck(void)
{
  SDA_OUT;
  SDA_LOW;
  delay_us(4);
  SCL_LOW;    
  delay_us(4);
  SCL_HIGH;
  delay_us(4);
  SCL_LOW;
  SDA_HIGH;
}


void I2C_SendNoAck(void)
{
  SDA_OUT;
  SDA_HIGH;
  delay_us(4);
  SCL_LOW;    
  delay_us(4);
  SCL_HIGH;
  delay_us(4);
  SCL_LOW;
}


unsigned char I2C_Check_ACK(void)
{
  unsigned char AckStatus;
  SDA_IN;
  SCL_HIGH;
  delay_us(4);
  if(P2IN & 0x04)
  {
    AckStatus = FALSE;
  }
  else
  {
    AckStatus = TRUE;
  }
  SCL_LOW;
  delay_us(2);
  SDA_OUT;
  return AckStatus;
}

void I2C_SendByte(unsigned char data)
{
  unsigned char tmp;
  SDA_OUT;
  for(tmp=0;tmp<8;tmp++)
  {
    if(data & 0x80)
    {
      SDA_HIGH;
    }
    else
    {
      SDA_LOW;
    }
    delay_us(4);
    SCL_HIGH;
    delay_us(4);
    SCL_LOW;
    delay_us(4);
    data <<= 1;
   }
  delay_us(10);
}

unsigned char I2C_RecvByte(void)
{
  unsigned char tmp;
  unsigned char DATA=0;
  SDA_IN;
  SCL_LOW;
  delay_us(4);
  for(tmp=0;tmp<8;tmp++)
  {
    SCL_HIGH;
    delay_us(4);
    DATA <<= 1;
    if(P2IN & 0x04)
    {
      DATA |= 0x01;
    }
    else
    {
      DATA &= 0xfe;
    }
    SCL_LOW;
  }
  SDA_OUT;
  return DATA;
}




#ifndef __I2C_H
#define __I2C_H
#include "cc430x513x.h"

#define SDA_IN            P2DIR &=~BIT2
#define SDA_OUT           P2DIR |= BIT2
#define SDA_LOW           P2OUT &=~BIT2
#define SDA_HIGH          P2OUT |=BIT2
#define SCL_IN            P2DIR &=~BIT1
#define SCL_OUT           P2DIR |=BIT1
#define SCL_LOW           P2OUT &=~BIT1
#define SCL_HIGH          P2OUT |=BIT1 


#define   TRUE                1
#define   FALSE               0
#define   AckError            0x55
#define   OutOfRang           0xaa
#define   OutOfAddr           0xbb

void I2C_START(void);
void I2C_STOP(void);
void I2C_SendAck(void);
void I2C_SendNoAck(void);
unsigned char I2C_Check_ACK(void);
void I2C_SendByte(unsigned char data);
unsigned char I2C_RecvByte(void);


#endif

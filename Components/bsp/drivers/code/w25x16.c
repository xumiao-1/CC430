#include "w25x16.h"
#include "delay.h"

void SPI_FLASH_Init(void)
{
  PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs  
  P1MAP3 = PM_UCB0SIMO;                     // Map UCA0SIMO output to P1.3 
  P1MAP2 = PM_UCB0SOMI;                     // Map UCA0SOMI output to P1.2 
  P1MAP4 = PM_UCB0CLK;                      // Map UCA0CLK output to P1.4 
  PMAPPWD = 0;                              // Lock port mapping registers 
  
  P1DIR |= BIT7;                            // Set P1.6 as TX output P1.7 as CS
  P1SEL |= BIT2 + BIT3 + BIT4;                     // Select P1.5 & P1.5 to UART function P1.3 P1.2 P1.4 as SPI
  
  UCB0CTL1 |= UCSSEL_2+UCSWRST;                      // **Put state machine in reset**
  UCB0CTL0 |= UCMST+UCSYNC+UCCKPL+UCMSB;   // 3-pin, 8-bit SPI master Clock polarity high, MSB
  UCB0BR0 = 0x02;
  UCB0BR1 = 0;
  
  UCB0CTL1 &= ~UCSWRST;   
  CS_1;
}

void SPI_Send_Data( unsigned char data)      //SPI发送一个字节数据
{    
    
    UCB0TXBUF = data;
    while ((UCB0IFG & UCTXIFG)==0); 
    _NOP();
    _NOP();
    _NOP();
    _NOP();
    //切记！！！SPI与CPU是分开单独工作的，此处需留有一定时间（对应4M时钟），保证CPU等待发送完成
}


unsigned char SPI_Read_Data( void)           //SPI读取一个字节数据
{
    UCB0IFG &=~ UCRXIFG;             //清  URXIFG0
    while ((UCB0IFG & UCTXIFG)==0);                 
    UCB0TXBUF = 0xff;
    
    //while ((IFG1 & URXIFG0)==0);         
    //切记！！！SPI与CPU是分开单独工作的，此处需留有一定时间（对应4M时钟），保证CPU等待接受完成
    return(UCB0RXBUF);	
}


void ExtenalFlash_Write_Enable(void )  // 写使能
{  
   CS_0;
   SPI_Send_Data( Write_Enable );
   CS_1;
}

unsigned char ExtenalFlash_Read_Register(void )  // 读使能
{
   unsigned char i;
   CS_0;
   SPI_Send_Data( Register_1 );
   i=SPI_Read_Data( );
   CS_1;
   return(i);
}
void ExtenalFlash_Power_Down(void )  // 进休眠模式
{
   CS_0;
   SPI_Send_Data( Power_Down );
   CS_1;
   delay_us(5);
}
void ExtenalFlash_Power_Up(void )  // 退出休眠模式
{
   CS_0;
   SPI_Send_Data( Power_Up );
   CS_1;
   delay_us(5);

}


void ExtenalFlash_Sector_Erase( long Adress )  //flash 擦除操作
{
        while((ExtenalFlash_Read_Register() & 0x01)==0x01);   // check busy ?
        ExtenalFlash_Write_Enable();
        
        CS_0;
        SPI_Send_Data( Sector_Erase );   //擦写使能
	SPI_Send_Data( (unsigned char)( Adress >>16) );     //MSB------> LSB 
	SPI_Send_Data( (unsigned char)( Adress >> 8) ); 
	SPI_Send_Data( (unsigned char)( Adress >> 0) );     
        CS_1;     //延时400ms才生效
        delay_ms(100) ;   
        
}


void ExtenalFlash_Write(unsigned char *p, int count ,long addr)  //flash 写操作
{
        int i = 0;

        while((ExtenalFlash_Read_Register() & 0x01)==0x01);// check busy ?
        ExtenalFlash_Write_Enable();
      
        CS_0;
        SPI_Send_Data( Page_Write );
	SPI_Send_Data( (unsigned char)( addr >>16) );     //MSB------> LSB 
	SPI_Send_Data( (unsigned char)( addr >> 8) ); 
	SPI_Send_Data( (unsigned char)( addr >> 0) );     
	for(i=0;i<count;i++)
        {
            UCB0TXBUF = *(p+i);
            while ((UCB0IFG & UCTXIFG)==0); 
            _NOP();
            _NOP();
            _NOP();
            _NOP();
            addr++;
            if((unsigned char)addr == 0)
            {
              CS_1;delay_ms(3);  //等待flash内部写操作完成
              if(addr %4096  ==0) //4KB擦除
                 ExtenalFlash_Sector_Erase( addr );
              while((ExtenalFlash_Read_Register() & 0x01)==0x01);// check busy ?
              ExtenalFlash_Write_Enable();
              CS_0;
              SPI_Send_Data( Page_Write );
              SPI_Send_Data( (unsigned char)( addr >>16) );     //MSB------> LSB 
              SPI_Send_Data( (unsigned char)( addr >> 8) ); 
              SPI_Send_Data( (unsigned char)( addr >> 0) );    
            }
        }
        CS_1;
        delay_ms(3);
   
}

void ExtenalFlash_Read(unsigned char *p, long addr , int count)  //flash 读操作
{
        int i = 0;
        
        while((ExtenalFlash_Read_Register() & 0x01)==0x01);// check busy ?
        CS_0;
        
        SPI_Send_Data( Read_Data );
	SPI_Send_Data( (unsigned char)( addr >>16) );     //MSB------> LSB 
	SPI_Send_Data( (unsigned char)( addr >> 8) ); 
	SPI_Send_Data( (unsigned char)( addr >> 0) );   
        SPI_Read_Data();
	for(i=0;i<count;i++)
        {
           *(p+i)=SPI_Read_Data( );    //读数据           
        }
        
        CS_1;
}

long ExtenalFlash_ReadID(void)  //读制造商ID
{
  long temp = 0,temp0 = 0,temp1 = 0,temp2=0;
  CS_0;
        
  SPI_Send_Data(JedecDeviceID);
  SPI_Read_Data();
  temp0 = SPI_Read_Data();
  temp1 = SPI_Read_Data();
  temp2 = SPI_Read_Data();
  CS_1;
  temp = (temp0<<16) | (temp1<<8) | temp2;
  return temp;
}

/*******************************************************************************
* Function Name  : SPI_FLASH_ReadID
* Description    : Reads FLASH identification.
* Input          : None
* Output         : None
* Return         : FLASH identification
*******************************************************************************/
unsigned char SPI_FLASH_ReadDeviceID(void)
{
  unsigned char Temp = 0;

  /* Select the FLASH: Chip Select low */
  CS_0;

  /* Send "RDID " instruction */
  SPI_Send_Data(DeviceID);
  SPI_Send_Data(Dummy_Byte);
  SPI_Send_Data(Dummy_Byte);
  SPI_Send_Data(Dummy_Byte);
  SPI_Read_Data();
  /* Read a byte from the FLASH */
  Temp = SPI_Read_Data();

  /* Deselect the FLASH: Chip Select high */
  CS_1;

  return Temp;
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BulkErase
* Description    : Erases the entire FLASH.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BulkErase(void)
{
  /* Send write enable instruction */
  ExtenalFlash_Write_Enable();

  /* Bulk Erase */
  /* Select the FLASH: Chip Select low */
  CS_0;
  /* Send Bulk Erase instruction  */
  SPI_Send_Data(Chip_Erase);
  /* Deselect the FLASH: Chip Select high */
  CS_1;

  /* Wait the end of Flash writing */
  SPI_FLASH_WaitForWriteEnd();
}


/*******************************************************************************
* Function Name  : SPI_FLASH_WaitForWriteEnd
* Description    : Polls the status of the Write In Progress (WIP) flag in the
*                  FLASH's status  register  and  loop  until write  opertaion
*                  has completed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_WaitForWriteEnd(void)
{
  unsigned char FLASH_Status = 0;

  /* Select the FLASH: Chip Select low */
  CS_0;

  /* Send "Read Status Register" instruction */
  SPI_Send_Data(Read_StatusReg);

  /* Loop as long as the memory is busy with a write cycle */
  do
  {
    /* Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    FLASH_Status = SPI_Read_Data();

  }
  while ((FLASH_Status & WIP_Flag) == 1); /* Write in progress */

  /* Deselect the FLASH: Chip Select high */
  CS_1;
}

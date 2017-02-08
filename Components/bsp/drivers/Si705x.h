#ifndef __SI705X_H
#define __SI705X_H

#define MeasureTemp_HoldMasterMode       0xE3
#define MeasureTemp_NoHoldMasterMode     0xF3
#define Reset                            0xFE
#define WriteRegister1                   0xE6
#define ReadRegister1                    0xE7
#define ReadElecIDByte1                  0xFA0F
#define ReadElecIDByte2                  0xFCC9
#define ReadFirmRev                      0x84B8
#define SlaveAddr                        0x80

void Si705x_Init(void);
unsigned char ReadUserRegister(void);
void WriteUserRegister(unsigned char data);
unsigned int getTemperature(void);
unsigned char ReadFrimwareRev(void);
#endif

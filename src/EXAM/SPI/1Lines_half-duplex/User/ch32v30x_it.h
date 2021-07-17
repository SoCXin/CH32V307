/********************************** (C) COPYRIGHT *******************************
* File Name          : ch32v30x_it.h
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : This file contains the headers of the interrupt handlers.
*******************************************************************************/
#ifndef __CH32V30x_IT_H
#define __CH32V30x_IT_H

#include "debug.h"

/* SPI Mode Definition */
#define HOST_MODE   0
#define SLAVE_MODE   1

/* SPI Communication Mode Selection */
#define SPI_MODE   HOST_MODE
//#define SPI_MODE   SLAVE_MODE

#define Size 18

extern volatile u8 Txval;
extern volatile u8 Rxval;
extern u16 TxData[Size];
extern u16 RxData[Size];

#endif /* __CH32V30x_IT_H */



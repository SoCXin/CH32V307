/*
 * File      : board.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-09-22     Bernard      add board.h to this bsp
 * 2017-10-20     ZYH          emmm...setup for HAL Libraries
 */

// <<< Use Configuration Wizard in Context Menu >>>
#ifndef __BOARD_H__
#define __BOARD_H__

#include "ch32v30x.h"
#define CH32V30X_PIN_NUMBERS   100
/* board configuration */
#define SRAM_SIZE  128
/* whether use board external SRAM memory */
// <e>Use external SRAM memory on the board
// 	<i>Enable External SRAM memory
#define EXT_SRAM 0
//	<o>Begin Address of External SRAM
//		<i>Default: 0x68000000
#define EXT_SRAM_BEGIN 0x68000000 /* the begining address of external SRAM */
//	<o>End Address of External SRAM
//		<i>Default: 0x68080000
#define EXT_SRAM_END 0x68080000 /* the end address of external SRAM */
// </e>
// <o> Internal SRAM memory size[Kbytes] <8-64>
//	<i>Default: 64
#define SRAM_END (0x20000000 + SRAM_SIZE * 1024)

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN  ((void *)&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define HEAP_BEGIN  (__segment_end("HEAP"))
#else
extern int _ebss;
#define HEAP_BEGIN  ((void *)&_ebss)
#endif
#define HEAP_END    SRAM_END


//extern volatile unsigned long  interrupter_sp_saver;
void rt_hw_board_init(void);

#endif /* __BOARD_H__ */

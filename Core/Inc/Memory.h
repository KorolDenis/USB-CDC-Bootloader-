/*
 * Memory.h
 *
 *  Created on: Sep 14, 2021
 *      Author: Денис
 */

#ifndef INC_MEMORY_H_
#define INC_MEMORY_H_

#define APPLICATION_ADDRESS    0x08003C00
#define FLASH_END              0x0801FFFF

// Команды
#define START_FLASH_PROGRAMM  1
#define FLASH_PROGRAMM        2
#define STOP_FLASH_PROGRAMM   3
#define ERASE                 4
#define START_FLASH_READ      5
#define FLASH_READ            6
#define STOP_FLASH_READ       7


void Decode_USB_Packet(void);
#endif /* INC_MEMORY_H_ */

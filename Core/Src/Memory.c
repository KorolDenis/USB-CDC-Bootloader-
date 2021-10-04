/*
 * Memory.c
 *
 *  Created on: Sep 14, 2021
 *      Author: Денис
 */
#include "stm32f1xx_hal.h"
#include "Memory.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

union
{
	uint32_t word;
	uint8_t bytes[4];
}Convert;

uint32_t Current_Address;
uint8_t Read_end_flag=0;

extern uint8_t UserTxBufferFS[64];
extern uint8_t UserRxBufferFS[64];

void Start_Flash_Programm(void)
{
  HAL_FLASH_Unlock();
  Current_Address=APPLICATION_ADDRESS;
  UserTxBufferFS[0]=START_FLASH_PROGRAMM;
  CDC_Transmit_FS(UserTxBufferFS,1);
}

void Flash_Programm(void)
{
    for(uint8_t i=0;i<15;i++)
    {
	 Convert.bytes[0]=UserRxBufferFS[i*4+1];
     Convert.bytes[1]=UserRxBufferFS[i*4+2];
     Convert.bytes[2]=UserRxBufferFS[i*4+3];
     Convert.bytes[3]=UserRxBufferFS[i*4+4];
	 HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Current_Address, Convert.word);
	 Current_Address+=4;
    }
    // как только зашили 15 слов, отвечаем подтверждением.
    UserTxBufferFS[0]=FLASH_PROGRAMM;
    CDC_Transmit_FS(UserTxBufferFS,64);
    UserTxBufferFS[0]=0;
}

void Stop_Flash_Programm(void)
{
   HAL_FLASH_Lock();
   Current_Address=APPLICATION_ADDRESS; // возвращаем на место адрес
   UserTxBufferFS[0]=STOP_FLASH_PROGRAMM;
   CDC_Transmit_FS(UserTxBufferFS,64);
   UserTxBufferFS[0]=0;
}
void Erase_Application(void)
{
	uint32_t CheckError;
	FLASH_EraseInitTypeDef Erase_init;
	HAL_FLASH_Unlock();
    Erase_init.TypeErase=FLASH_TYPEERASE_PAGES;
    Erase_init.PageAddress=APPLICATION_ADDRESS;
    Erase_init.NbPages=(FLASH_END-APPLICATION_ADDRESS)/FLASH_PAGE_SIZE;
    HAL_FLASHEx_Erase(&Erase_init, &CheckError);
    HAL_FLASH_Lock();
    if(CheckError==0xFFFFFFFF) // если всё стерлось
    {
    	   UserTxBufferFS[0]=ERASE;
    	   CDC_Transmit_FS(UserTxBufferFS,64);
    	   UserTxBufferFS[0]=0;
    }
}
void Start_Flash_Read(void)
{
	HAL_FLASH_Unlock();
	Read_end_flag=0;
	Current_Address=APPLICATION_ADDRESS;
	UserTxBufferFS[0]=START_FLASH_READ;
	CDC_Transmit_FS(UserTxBufferFS,1);
	//Flash_Read();
}
void Flash_Read(void)
{
   uint32_t *p;

   if(Read_end_flag)
   {
	   Stop_Flash_Read();
	   return;
   }
   for(uint8_t i=0;i<15;i++)
   {
   p=Current_Address;
   Convert.word= *p; // содержимое по адресу берем
   UserTxBufferFS[i*4+1]=Convert.bytes[0];
   UserTxBufferFS[i*4+2]=Convert.bytes[1];
   UserTxBufferFS[i*4+3]=Convert.bytes[2];
   UserTxBufferFS[i*4+4]=Convert.bytes[3];

   Current_Address+=4;
   if(Convert.word==0xFFFFFFFF)
   {
	   Read_end_flag=1;
	   break;
   }
   }
   UserTxBufferFS[0]=FLASH_READ;
   CDC_Transmit_FS(UserTxBufferFS,64);
   UserTxBufferFS[0]=0;
}
void Stop_Flash_Read(void)
{
	HAL_FLASH_Lock();
	Read_end_flag=0;
	Current_Address=APPLICATION_ADDRESS;
	UserTxBufferFS[0]=STOP_FLASH_READ;
	CDC_Transmit_FS(UserTxBufferFS,1);
}
void Decode_USB_Packet(void)
{

  if(UserRxBufferFS[0]==START_FLASH_PROGRAMM) Start_Flash_Programm();
  if(UserRxBufferFS[0]==FLASH_PROGRAMM) Flash_Programm();
  if(UserRxBufferFS[0]==STOP_FLASH_PROGRAMM) Stop_Flash_Programm();
  if(UserRxBufferFS[0]==ERASE) Erase_Application();
  if(UserRxBufferFS[0]==START_FLASH_READ) Start_Flash_Read();
  if(UserRxBufferFS[0]==FLASH_READ) Flash_Read();
  if(UserRxBufferFS[0]==STOP_FLASH_READ) Stop_Flash_Read();
}

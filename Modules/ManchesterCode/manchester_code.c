/**
  ******************************************************************************
  * @file           : manchester_code.c
  * @brief          : Manchester code driver
  * @author         : MicroTechnics (microtechnics.ru)
  ******************************************************************************
  */



/* Includes ------------------------------------------------------------------*/

#include "manchester_code.h"



/* Declarations and definitions ----------------------------------------------*/

static uint8_t virtTact = 1;
static MANCH_Data encodeData;
static uint16_t encodeTimerCnt = 0;

extern TIM_HandleTypeDef htim2;



/* Functions -----------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static void SetOutput(uint8_t state)
{
  HAL_GPIO_WritePin(MANCH_OUTPUT_PORT, MANCH_OUTPUT_PIN, (GPIO_PinState)state);
}



/*----------------------------------------------------------------------------*/
void MANCH_Encode(uint8_t* data, uint8_t size)
{
  encodeData.bitIdx = 0;
  encodeData.byteIdx = 0;
  
  if (size > MANCH_DATA_BYTES_NUM)
  {
    encodeData.bytesNum = MANCH_DATA_BYTES_NUM + MANCH_SYNC_BYTES_NUM;
  }
  else
  {
    encodeData.bytesNum = size + MANCH_SYNC_BYTES_NUM;
  }
  
  memcpy(&encodeData.data[MANCH_SYNC_BYTES_NUM], data, encodeData.bytesNum - MANCH_SYNC_BYTES_NUM);
  encodeData.data[0] = MANCH_SYNC_FIELD & 0xFF;
  encodeData.data[1] = (MANCH_SYNC_FIELD & 0xFF00) >> 8;
  
  encodeTimerCnt = 0;
  virtTact = 1;
  encodeData.active = 1;
}



/*----------------------------------------------------------------------------*/
static uint8_t GetDataBit(MANCH_Data* manchData)
{
  uint8_t res;
  
  uint8_t curByte = manchData->data[manchData->byteIdx];
  uint8_t curBitIdx = manchData->bitIdx;
  
  res = (curByte >> curBitIdx) & 0x01;
  
  return res;
}



/*----------------------------------------------------------------------------*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim2)
  {
    // Encoding process
    if (encodeData.active == 1)
    {
      if ((encodeTimerCnt == (MANCH_ENCODE_TIMER_MAX / 2)) ||
          (encodeTimerCnt == MANCH_ENCODE_TIMER_MAX))
      {
        uint8_t curCodeBit = GetDataBit(&encodeData);
        uint8_t curOutputBit = curCodeBit ^ virtTact;
        SetOutput(curOutputBit);
        virtTact ^= 0x01;
      }
            
      if (encodeTimerCnt == MANCH_ENCODE_TIMER_MAX)
      {        
        encodeData.bitIdx++;
        
        if (encodeData.bitIdx == (MANCH_BITS_IN_BYTE_NUM))
        {
          encodeData.bitIdx = 0;
          
          encodeData.byteIdx++;
          if (encodeData.byteIdx == encodeData.bytesNum)
          {
            encodeData.active = 0;
          }
        }
        
        encodeTimerCnt = 0;
      }
    
      encodeTimerCnt++;
    }
  }
}



/*----------------------------------------------------------------------------*/

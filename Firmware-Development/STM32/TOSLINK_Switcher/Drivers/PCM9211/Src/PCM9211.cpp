/*
 * PCM9211.cpp
 *
 *  Created on: May 23, 2024
 *      Author: Syed Farhan Ahmed
 */

/* Includes ------------------------------------------------------------------*/

#include <PCM9211.hpp>


/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define PCM9211_TIMOUT_TICK       100
#define PCM9211_MEM_ADDRESS_SIZE   1

#define PCM9211_OK                 0
#define PCM9211_IO_ERROR           1

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

void PCM9211_Reset(void)
{
  HAL_GPIO_WritePin(RESET_GPIO_Port, RESET_Pin, GPIO_PIN_SET); // reset PCM IC
  HAL_Delay(100);
  HAL_GPIO_WritePin(RESET_GPIO_Port, RESET_Pin, GPIO_PIN_RESET); //
  HAL_Delay(100);
}

PCM9211::PCM9211(SPI_HandleTypeDef* spi, PCM9211_Mode_t mode, SS_Conf* SS)
//	: _spi(spi), _mode(mode)
{
	// Contructor called, object created
	_spi = spi;
	_mode = mode;
	_SS_port = SS->port;
	_SS_pin = SS->pin;
}

PCM9211::~PCM9211() {
	// TODO Auto-generated destructor stub
}

#ifdef	i2c_mode
static I2C_HandleTypeDef *_i2c;
uint8_t PCM9211_Init(I2C_HandleTypeDef *i2c, uint8_t address)
{
  PCM9211_Reset();
  _i2c = i2c;
  _devAddress = address;
  uint8_t readValue = 0;

  /*** IO Self Test ***/
  Write(0x2E,0xAA);
   readValue = Read(0x2E);
  if(readValue != 0xAA)
    return PCM9211_IO_ERROR;
  Write(0x2E,0x00);

  /*** Soft Reset ***/
  Write(0x40, 0x00);
  HAL_Delay(100);
  readValue = Read(0x40);
  if(readValue != 0xC0)
    return PCM9211_IO_ERROR;

  /*** Disable: ADC, DIT ***/
  Write(0x40, 0xE2);
  readValue = Read(0x40);
  if(readValue != 0xE2)
    return PCM9211_IO_ERROR;

  PCM9211_SelectSource(PCM9211_RXIN0);

  /*** Main Output Source: DIR ***/
  Write(0x6B, 0x11);

  return PCM9211_OK;
}
#endif

#ifndef	i2c_mode
uint8_t PCM9211::PCM9211_Init()//SPI_HandleTypeDef *spi, PCM9211_Mode_t mode)
{
//  PCM9211_Reset();
//  _spi = spi;
//  _mode = mode;

//  PCM9211_SS(PCM_SS_SELECT);		// select SS pin
  uint8_t readValue = 0;

  /*** IO Self Test ***/
  Write_(0x2E,0xAA);
   readValue = Read_(0x2E);
  if(readValue != 0xAA)
    return PCM9211_IO_ERROR;
  Write_(0x2E,0x00);

  /*** Soft Reset ***/
  Write_(0x40, 0x00);
  HAL_Delay(100);
  readValue = Read_(0x40);
  if(readValue != 0xC0)
    return PCM9211_IO_ERROR;

  switch(_mode){
  case PCM_RX:
	  /*** Disable: ADC, DIT ***/
	  Write_(0x40, 0xE2);
	  readValue = Read_(0x40);
	  if(readValue != 0xE2)
		return PCM9211_IO_ERROR;
	  /*** Main Output Source: DIR ***/
	  Write_(0x6B, 0x11);
	  readValue = Read_(0x6B);
	  if(readValue != 0x11)
		return PCM9211_IO_ERROR;
	  break;
  case PCM_TX:
	  /*** Disable: ADC, DIR ***/
	  Write_(0x40, 0xF0);
	  readValue = Read_(0x40);
	  if(readValue != 0xF0)
		return PCM9211_IO_ERROR;
	  /*** Main Output Source: DIT <- AUXIN0 ***/
	  Write_(0x60, 0x33);
	  readValue = Read_(0x60);
	  if(readValue != 0x33)
		return PCM9211_IO_ERROR;
	  /*** MPO0 Output Source: MPO0 <- DIT ***/
	  Write_(0x78, 0x3D);
	  readValue = Read_(0x78);
	  if(readValue != 0x3D)
		return PCM9211_IO_ERROR;
	  break;
  default:
	  break;
  }

//  PCM9211_SelectSource(PCM9211_RXIN0);

  return PCM9211_OK;
}
#endif

/*
 * 0x00: RXIN0 - SPDIF0
 * 0x01: RXIN1 - SPDIF1
 * 0x02: RXIN2 - AES/EBU	//default
 * 0x03: RXIN3 - TOSLink
 */

void PCM9211::PCM9211_SelectSource(uint8_t source)
{
  /*** Eanble RXIN0(SPDIF), RXIN1(TOSlink) amplifier and select source***/
  source &= 0x0F;
  Write_(0x34, source);
}

/*
0000: Out of range
0001: 8 kHz
0010: 11.025 kHz
0011: 12 kHz
0100: 16 kHz
0101: 22.05 kHz
0110: 24 kHz
0111: 32 kHz
1000: 44.1 kHz
1001: 48 kHz
1010: 64 kHz
1011: 88.2 kHz
1100: 96 kHz
1101: 128 kHz
1110: 176.4 kHz
1111: 192 kHz
*/
//PCM9211_Frequencies_t PCM9211::PCM9211_SamplingFreq(void)
uint8_t PCM9211::PCM9211_SamplingFreq(void)
{
  uint8_t readValue = 0;
  uint8_t freq = 0;
  readValue = Read_(0x38);
  if((readValue & 0x80) == 0)//Calculated
  {
    if((readValue & 0x70)==0) //DIR - Port
    {
      freq = readValue & 0x0F;
    }
  }
  return freq;
}




uint32_t PCM9211::PCM9211_DataInfromation()
{

  return Read_(0x3A);
 // return ReadU32(0x3A);
}


#ifdef	i2c_mode
static uint8_t Read(uint8_t reg)
{
  uint8_t value = 0;
  if(HAL_I2C_MemRead_(_i2c, _devAddress, reg, PCM9211_MEM_ADDRESS_SIZE, &value, sizeof(value), PCM9211_TIMOUT_TICK)!= HAL_OK)
    _errorCounter++;
  return value;
}

static uint32_t ReadU32(uint8_t reg)
{
  uint32_t value = 0;
  uint8_t temp[4];

  if(HAL_I2C_MemRead_(_i2c, _devAddress, reg, PCM9211_MEM_ADDRESS_SIZE, temp, sizeof(temp), PCM9211_TIMOUT_TICK)!= HAL_OK)
    _errorCounter++;
  value = *((uint32_t*)temp);
  return value;
}

static uint8_t Write(uint8_t reg, uint8_t value)
{
  if(HAL_I2C_MemWrite_(_i2c, _devAddress, reg, PCM9211_MEM_ADDRESS_SIZE, &value, sizeof(value), PCM9211_TIMOUT_TICK)!= HAL_OK)
    _errorCounter++;
  return HAL_OK;
}
#endif

#ifndef	i2c_mode
uint8_t PCM9211::Read_(uint8_t reg)
{
    uint8_t txData[1];
    uint8_t rxData[1];

//    txData[0] = reg & 0x7F;  // Clear MSB for read operation
    txData[0] = reg | 0x80;  // Set MSB for read operation

    PCM9211_SS(PCM_SS_SELECT);
    HAL_SPI_Transmit(_spi, txData, 1, HAL_MAX_DELAY);  // Transmit register address
    HAL_SPI_Receive(_spi, rxData, 1, HAL_MAX_DELAY);   // Receive data
    PCM9211_SS(PCM_SS_DESELECT);

    return rxData[0];
}

void PCM9211::Write_(uint8_t reg, uint8_t data)
{
    uint8_t txData[2];
//    txData[0] = reg | 0x80;  // Set MSB for write operation
    txData[0] = reg & 0x7F;  // Clear MSB for write operation
    txData[1] = data;

    PCM9211_SS(PCM_SS_SELECT);
    HAL_SPI_Transmit(_spi, txData, 2, HAL_MAX_DELAY);  // Transmit register address and data
    PCM9211_SS(PCM_SS_DESELECT);
}

void PCM9211::PCM9211_SS(PCM9211_Mode_t state)
{
	switch(state){
		case PCM_SS_SELECT:
		  HAL_GPIO_WritePin(_SS_port, _SS_pin, GPIO_PIN_RESET);
		  break;
		case PCM_SS_DESELECT:
		  HAL_GPIO_WritePin(_SS_port, _SS_pin, GPIO_PIN_SET);
		  break;
		default:
			break;
	}
}
#endif


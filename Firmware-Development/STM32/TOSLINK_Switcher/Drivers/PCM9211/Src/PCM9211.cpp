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
  HAL_GPIO_WritePin(RESET_GPIO_Port, RESET_Pin, GPIO_PIN_RESET); // reset PCM IC
  HAL_Delay(100);
  HAL_GPIO_WritePin(RESET_GPIO_Port, RESET_Pin, GPIO_PIN_SET); //
  HAL_Delay(100);
}

PCM9211::PCM9211(SPI_HandleTypeDef* spi, PCM9211_Mode_t mode, SS_Conf* SS, CHANNEL_BUFFERS_t* buffers)
//	: _spi(spi), _mode(mode)
{
	// Contructor called, object created
	printf("PCM9211 Constructor called.\r\n");

	_spi = spi;
	_mode = mode;
	_SS_port = SS->port;
	_SS_pin = SS->pin;
	_buffers = buffers;
	_isReset = false;
}

PCM9211::~PCM9211() {
	// TODO Auto-generated destructor stub
}

uint8_t PCM9211::PCM9211_Init(uint8_t state)
{
  uint8_t readValue = 0;

  /*** IO Self Test ***/
  if(Write_REG(0x2E, 0xAA))
	return PCM9211_IO_ERROR;
  Write_(0x2E,0x00);
//  Write_regMask(0x2E,0x00);

  /*** Soft Reset ***/
  Write_(0x40, REGMASK & (0x00));
  HAL_Delay(100);
  Write_(0x40, REGMASK & (REG_ADDIS | REG_RXDIS | REG_TXDIS | REG_XODIS));//0x33);
  HAL_Delay(100);
  readValue = Read_(0x40);
  if(readValue != (REG_MRST | REG_SRST))//0xC0)
	return PCM9211_IO_ERROR;

  switch(state){
  case 0:
	/*** XTI CLK source 12.288 and BCK 3.072, LRCK 48k = XTI/512: ***/
		  if(Write_REG(0x31, REGMASK & (REG_XSCK_12_288 | REG_XBCK_3_072 | REG_XLRCK_48)))//0x1A))
			return PCM9211_IO_ERROR;

//		  Write_(0x33, 0x22);
//		  readValue = Read_(0x33);
//		  if(readValue != 0x22)
//			return PCM9211_IO_ERROR;

		  if(Write_REG(0x20, REGMASK & (0x00)))
			return PCM9211_IO_ERROR;

		  if(Write_REG(0x24, REGMASK & (0x00)))
			return PCM9211_IO_ERROR;

		  if(Write_REG(0x26, REGMASK & (0x81)))
			return PCM9211_IO_ERROR;

		  if(Write_REG(0x33, REGMASK & (0x22)))
			return PCM9211_IO_ERROR;

	  switch(_mode){
	  case PCM_RX:
		  /*** Disable: ADC, DIR ***/
//		  Write_(0x40, (REG_MRST | REG_SRST | REG_ADDIS | REG_RXDIS)	//0xF0);							// ADC/DIR turned off
//		  Write_(0x40, (REG_MRST | REG_SRST | REG_ADDIS | REG_RXDIS | REG_TXDIS | REG_XODIS)	//0xF3);	// turned off everything, power down
//		  Write_(0x40, (REG_MRST | REG_SRST)	//0xC0);													// all normal operation, power up
		  if(Write_REG(0x40, REGMASK & (REG_MRST | REG_SRST | REG_ADDIS | REG_TXDIS)))//0xC0))
			return PCM9211_IO_ERROR;
	/*** ---------Start DIR settings----------: ***/
		  /*** DIR Receivable Incoming Biphase's Sampling Frequency Range Setting: ***/
		  if(Write_REG(0x21, REGMASK & (REG_RXFSRNG)))
			return PCM9211_IO_ERROR;

		  /*** DIR CLKSTP and VOUT delay: ***/
		  if(Write_REG(0x22, REGMASK & (REG_RXVDLY)))//0x01))
			return PCM9211_IO_ERROR;

		  /*** DIR OCS start up wait time and Process for Parity Error Detection and ERROR Release Wait Time Setting: ***/
		  if(Write_REG(0x23, REGMASK & (REG_XTIWT_25 | REG_PRTPRO_PCMonly | REG_ERRWT_48)))//0x04))
			return PCM9211_IO_ERROR;

		  /*** DIR Acceptable fs Range Setting & Mask: ***/
		  if(Write_REG(0x27, 0x00))
			return PCM9211_IO_ERROR;

		  /*** DIR Output Data Format, 24bit I2S mode: ***/
		  if(Write_REG(0x2F, REGMASK & (REG_RXFMT_24_i2s)))//0x04))
			return PCM9211_IO_ERROR;

		  /*** DIR Recovered System Clock (SCK) Ratio Setting: ***/
		  if(Write_REG(0x30, 0x02))
			return PCM9211_IO_ERROR;

		  /*** DIR Source, Secondary Bit/LR Clock (SBCK/SLRCK) Frequency Setting: ***/
		  if(Write_REG(0x32, REGMASK & (REG_PSBCK_64 | REG_PSLRCK_1fs)))//0x22))
			return PCM9211_IO_ERROR;

		  /*** DIR Input Biphase Signal Source Select and RXIN02: ***/
		  /*** PWR down amplifier, Select RXIN2: ***/
		  if(Write_REG(0x34, REGMASK & (REG_RX0DIS | REG_RX1DIS | REG_RXSEL_RXIN2)))//0xC2))
			return PCM9211_IO_ERROR;

		  /*** Port Sampling Frequency Calculator Measurement Target Setting, Cal and DIR Fs: ***/
//		  Write_(0x37, 0x05);	// MAIN output port selected
////		  Write_(0x37, 0x00);	// DIR selected
		  if(Write_REG(0x37, REGMASK & (REG_PFSTGT_MAIN_OUTPUT)))//0x05))
			return PCM9211_IO_ERROR;
	/*** ---------End DIR settings----------: ***/

	/*** ---------Start  MainOutput Settings----------: ***/
		  /*** Main Output & AUXOUT Port Control: ***/
		  if(Write_REG(0x6A, 0x00))
			return PCM9211_IO_ERROR;

		  /*** Main Output Port (SCKO/BCK/LRCK/DOUT) Source Setting: ***/
		  if(Write_REG(0x6B, REGMASK & (REG_MOSSRC_DIR | REG_MOPSRC_DIR)))//0x11))
			return PCM9211_IO_ERROR;

		  /*** MPIO_B & Main Output Port Hi-Z Control: ***/
		  if(Write_REG(0x6D, 0x00))
			return PCM9211_IO_ERROR;
	/*** ---------End  MainOutput Settings----------: ***/
		  break;
	  case PCM_TX:
		  /*** Disable: ADC, DIR ***/
//		  Write_(0x40, (REG_MRST | REG_SRST | REG_ADDIS | REG_RXDIS)	//0xF0);							// ADC/DIR turned off
//		  Write_(0x40, (REG_MRST | REG_SRST | REG_ADDIS | REG_RXDIS | REG_TXDIS | REG_XODIS)	//0xF3);	// turned off everything, power down
//		  Write_(0x40, (REG_MRST | REG_SRST)	//0xC0);													// all normal operation, power up
		  if(Write_REG(0x40, REGMASK & (REG_MRST | REG_SRST)))//0xC0))
			return PCM9211_IO_ERROR;

//		  /*** ERROR port: ***/
//		  Write_(0x20, 0x05);
//		  if(Write_REG(0x20, REGMASK & (REG_ERRSEL | REG_NPCMSEL)))//0x05))
//			return PCM9211_IO_ERROR;

		  /*** XTI Clock enable (not muted):***/
		  if(Write_REG(0x24, 0x10))
			return PCM9211_IO_ERROR;

		  /*** Port Sampling Frequency Calculator Measurement Target Setting:***/
//		  Write_(0x37, 0x02);	// AUXIN0
//		  Write_(0x37, 0x07);	// DIT
		  if(Write_REG(0x37, REGMASK & (REG_PFSTGT_AUXIN0)))//0x02))
			return PCM9211_IO_ERROR;

//		  /*** Main Input Source: DIT <- AUXIN0 ***/
//		  Write_(0x60, 0x33);	// DIT clock and audio; AUXIN0
//		  Write_(0x60, 0x23);	// DIT audio; AUXIN0, DIT clock; ADC
//		  if(Write_REG(0x60, REGMASK & (REG_TXSSRC_ADC | REG_TXPSRC_AUXIN0)))//0x23))
		  if(Write_REG(0x60, REGMASK & (REG_TXSSRC_AUXIN0 | REG_TXPSRC_AUXIN0)))//0x33))
			return PCM9211_IO_ERROR;

		  /*** DIT Function Control 2/3: ***/
//		  Write_(0x61, 0x00);		//0x10 default
//		  Write_(0x61, 0x10);		//0x10 default
		  if(Write_REG(0x61, REGMASK & (REG_TXSCK_256 | REG_TXFMT_i2s)))//0x10))
			return PCM9211_IO_ERROR;

//		  /*** Main Output Port (SCKO/BCK/LRCK/DOUT) Source Setting: ***/
//		  if(Write_REG(0x6B, REGMASK & (REG_MOSSRC_AUXIN0 | REG_MOPSRC_AUXIN0)))//0x33))		// AUXIN0 -> Main Port
//			return PCM9211_IO_ERROR;

		  //		  /*** AUX output port: AUXOUT <- AUXIN0 ***/
//		  Write_(0x6C, 0x33);
//		  readValue = Read_(0x6C);
//		  if(readValue != 0x33)
//			return PCM9211_IO_ERROR;

//		  /*** RECOUT0 Output Biphase Source Settings: ***/
//		  if(Write_REG(0x35, 0x0F))	// TXOUT (internal DIT output)
//			return PCM9211_IO_ERROR;

		  /*** MPO0 Output Source: MPO0 <- DIT ***/
//		  Write_(0x78, 0xEE);	// MPO0 <- RECOUT0
//		  Write_(0x78, 0xDD);	// MPO0 <- DIT
		  if(Write_REG(0x78, REGMASK & (REG_MPO1SEL_TXOUT | REG_MPO0SEL_TXOUT)))//0xDD))
			return PCM9211_IO_ERROR;
		  break;
	  default:
		  break;
	  }
	  break;
  case 1:	//disable the PCM components
	/*** Disable: ADC, DIR ***/
	  if(Write_REG(0x40, 0xF3))	// turned off everything
		  return PCM9211_IO_ERROR;
	  break;
  default:
	  break;
  }

//	  PCM9211_SelectSource(PCM9211_RXIN0);

  return PCM9211_OK;
}

uint8_t PCM9211::Channel_Buffers_Set(void)
{
  switch(_mode){
  case PCM_RX:
	  break;
  case PCM_TX:
	  Write_REG(0x63, _buffers->REG_A);
	  Write_REG(0x64, _buffers->REG_B);
	  Write_REG(0x65, _buffers->REG_C);
	  Write_REG(0x66, _buffers->REG_D);
	  Write_REG(0x67, _buffers->REG_E);
	  Write_REG(0x68, _buffers->REG_F);
	  break;
  default:
	  return PCM9211_IO_ERROR;
	  break;
  }

  return PCM9211_OK;
}

uint8_t PCM9211::Channel_Buffers_Get(void)
{
  switch(_mode){
  case PCM_RX:
	  _buffers->REG_A = Read_(0x5A);
	  _buffers->REG_B = Read_(0x5B);
	  _buffers->REG_C = Read_(0x5C);
	  _buffers->REG_D = Read_(0x5D);
	  _buffers->REG_E = Read_(0x5E);
	  _buffers->REG_F = Read_(0x5F);
	  break;
  case PCM_TX:
	  _buffers->REG_A = Read_(0x63);
	  _buffers->REG_B = Read_(0x64);
	  _buffers->REG_C = Read_(0x65);
	  _buffers->REG_D = Read_(0x66);
	  _buffers->REG_E = Read_(0x67);
	  _buffers->REG_F = Read_(0x68);
	  break;
  default:
	  return PCM9211_IO_ERROR;
	  break;
  }

  return PCM9211_OK;
}

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
  this->Write_(0x34, source);
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
SAMPLING_FREQ_INFO_t PCM9211::PCM9211_SamplingFreq(void)
{
  SAMPLING_FREQ_INFO_t regValue;
  int8_t readValue = 0;

  readValue = Read_(0x39);				// Incoming Biphase Information and Sampling Frequency
  if((readValue & 0x80) == 0)			// if calculated
  {
	  regValue.audio_39 = (readValue & 0x40) >> 6;		// 0 if Audio data, 1 if non audio
      regValue.freq_39 = (readValue & 0x0F);
  }

  readValue = Read_(0x38);				// Port Sampling Frequency Calculator
  if((readValue & 0x80) == 0)			// if calculated
  {
	  regValue.port_38 = (readValue & 0x70) >> 4;
      regValue.freq_38 = (readValue & 0x0F);
  }

  return regValue;
}




uint32_t PCM9211::PCM9211_DataInfromation()
{

  return Read_(0x3A);
 // return ReadU32(0x3A);
}




uint8_t PCM9211::Read_(uint8_t reg)
{
	HAL_StatusTypeDef errorcode;
    uint8_t txData[1];
    uint8_t rxData[1];

//    txData[0] = reg & 0x7F;  // Clear MSB for read operation
    txData[0] = reg | 0x80;  // Set MSB for read operation

    this->PCM9211_SS(PCM_SS_SELECT);
//    HAL_Delay(1);
    do{
    	errorcode = HAL_SPI_Transmit(_spi, txData, 1, HAL_MAX_DELAY);  // Transmit register address
	}while(errorcode != HAL_OK);
    do{
		HAL_SPI_Receive(_spi, rxData, 1, HAL_MAX_DELAY);   // Receive data
	}while(errorcode != HAL_OK);
    this->PCM9211_SS(PCM_SS_DESELECT);

    return rxData[0];
}

uint8_t PCM9211::Write_REG(uint8_t reg, uint8_t data)
{
	uint8_t readValue;
	Write_(reg, data);	// DIT audio; AUXIN0, DIT clock; ADC
	HAL_Delay(1);
	readValue = Read_(reg);
	if(readValue != data)
		return PCM9211_IO_ERROR;
	else
		return PCM9211_OK;
}

void PCM9211::Write_(uint8_t reg, uint8_t data)
{
	HAL_StatusTypeDef errorcode;
    uint8_t txData[2];
//    txData[0] = reg | 0x80;  // Set MSB for write operation
    txData[0] = reg & 0x7F;  // Clear MSB for write operation
    txData[1] = data;


    this->PCM9211_SS(PCM_SS_SELECT);
//    HAL_Delay(1);
    do{
    	errorcode = HAL_SPI_Transmit(_spi, &txData[0], 1, HAL_MAX_DELAY);  // Transmit register address and data
    }while(errorcode != HAL_OK);
    do{
    	HAL_SPI_Transmit(_spi, &txData[1], 1, HAL_MAX_DELAY);  // Transmit register address and data
    }while(errorcode != HAL_OK);

    this->PCM9211_SS(PCM_SS_DESELECT);
}

void PCM9211::Write_regMask(uint8_t reg, uint8_t data)
{
    // read register and set only the bits that are required
    uint8_t txData;
    uint8_t regData;

    regData = Read_(reg);
    txData = data | regData;

    Write_(reg, txData);
}

void PCM9211::PCM9211_SS(PCM9211_Mode_t state)
{
	switch(state){
		case PCM_SS_SELECT:
		  HAL_GPIO_WritePin(this->_SS_port, this->_SS_pin, GPIO_PIN_RESET);
		  break;
		case PCM_SS_DESELECT:
		  HAL_GPIO_WritePin(this->_SS_port, this->_SS_pin, GPIO_PIN_SET);
		  break;
		default:
			break;
	}
}

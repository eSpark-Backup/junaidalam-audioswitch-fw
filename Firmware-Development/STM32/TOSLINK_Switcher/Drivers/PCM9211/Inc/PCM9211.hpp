/*
 * PCM9211.h
 *
 *  Created on: May 23, 2024
 *      Author: Syed Farhan Ahmed
 */

#ifndef PCM9211_INC_PCM9211_HPP_
#define PCM9211_INC_PCM9211_HPP_

#include <stdint.h>
#include "main.h"

//#define	i2c_mode	1				//un-comment this if you are using i2c communication

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
//	DIR input sources macros
#define PCM9211_RXIN0   0x00
#define PCM9211_RXIN1   0x01
#define PCM9211_RXIN2   0x02		//default
#define PCM9211_RXIN3   0x03

//	PCM mode enum
typedef enum{
	PCM_TX = 1,
	PCM_RX,
	PCM_SS_SELECT,
	PCM_SS_DESELECT
}PCM9211_Mode_t;

typedef struct{
	GPIO_TypeDef*	port;
	uint16_t		pin;
}SS_Conf;

typedef enum _PCM9211_Frequencies_t
{
  PCM9211_OUT_OF_RANGE = 0x00,
  PCM9211_8KHz,         //0x01
  PCM9211_11_025KHz,    //0x02
  PCM9211_12KHz,        //0x03
  PCM9211_16KHz,        //0x04
  PCM9211_22_05KHz,     //0x05
  PCM9211_24KHz,        //0x06
  PCM9211_32KHz,        //0x07
  PCM9211_44_1KHz,      //0x08
  PCM9211_48KHz,        //0x09
  PCM9211_64KHz,        //0x0A
  PCM9211_88_2KHz,      //0x0B
  PCM9211_96KHz,        //0x0C
  PCM9211_128KHz,       //0x0D
  PCM9211_176_4KHz,     //0x0E
  PCM9211_192KHz        //0x0F
}PCM9211_Frequencies_t;

/* Exported functions ------------------------------------------------------- */
void PCM9211_Reset(void);


class PCM9211{
public:
	PCM9211(SPI_HandleTypeDef* spi, PCM9211_Mode_t mode, SS_Conf* SS);
	virtual ~PCM9211();

//	uint8_t PCM9211_Init(I2C_HandleTypeDef *i2c, uint8_t address);
	uint8_t PCM9211_Init();//SPI_HandleTypeDef *spi, PCM9211_Mode_t mode);
private:
	void PCM9211_SelectSource(uint8_t source);
	uint8_t PCM9211_SamplingFreq(void);
	uint32_t PCM9211_DataInfromation();
	uint8_t Read_(uint8_t reg);
	void Write_(uint8_t reg, uint8_t data);
	void PCM9211_SS(PCM9211_Mode_t state);


	SPI_HandleTypeDef* _spi;
	PCM9211_Mode_t _mode;
	GPIO_TypeDef* _SS_port;
	uint16_t _SS_pin;
	bool _isReset;
};

#endif /* PCM9211_INC_PCM9211_HPP_ */

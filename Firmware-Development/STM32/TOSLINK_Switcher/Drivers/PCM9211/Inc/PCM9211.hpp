/*
 * PCM9211.h
 *
 *  Created on: May 23, 2024
 *      Author: Syed Farhan Ahmed
 */

#ifndef PCM9211_INC_PCM9211_HPP_
#define PCM9211_INC_PCM9211_HPP_

#include <stdint.h>
#include <stdio.h>
#include "main.h"

//#define	i2c_mode	1				//un-comment this if you are using i2c communication

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Register macros ------------------------------------------------------------*/

#define REGMASK					0xFF				// Should be (AND) by rest of the reg bits

// 20h
#define REG_ERRCON				0x40
#define REG_MCHR				0x20
#define REG_ERRHZ				0x08
#define REG_ERRSEL				0x04
#define REG_NPCMHZ				0x02
#define REG_NPCMSEL				0x01
// 21h
#define REG_RXFSRNG				0x10
// 22h
#define REG_CLKSTCON			0x40
#define REG_CLKSTP				0x10
#define REG_RXVDLY				0x01
// 23h
#define REG_XTIWT_25			0x00
#define REG_XTIWT_50			0x10
#define REG_XTIWT_100			0x20
#define REG_XTIWT_200			0x30
#define REG_PRTPRO_noProcess	0x00
#define REG_PRTPRO_PCMonly		0x04
#define REG_PRTPRO_PCMandNOPCM	0x08
#define REG_PRTPRO_reserved		0x0C
#define REG_ERRWT_48			0x00
#define REG_ERRWT_12			0x01
#define REG_ERRWT_6				0x02
#define REG_ERRWT_3				0x03
// 2Fh
#define REG_RXFMT_24bitRight	0x00
#define REG_RXFMT_24bitLeft		0x05
#define REG_RXFMT_16bitRight	0x03
#define REG_RXFMT_24_i2s		0x04
// 31h
#define REG_XSCK_24_576			0x00
#define REG_XSCK_12_288			0x10
#define REG_XSCK_6_144			0x20
#define REG_XSCK_3_072			0x30
#define REG_XBCK_12_288			0x00
#define REG_XBCK_6_144			0x04
#define REG_XBCK_3_072			0x08
#define REG_XBCK_1_536			0x09
#define REG_XLRCK_192			0x00
#define REG_XLRCK_96			0x01
#define REG_XLRCK_48			0x02
#define REG_XLRCK_24			0x03
// 32h
#define REG_PSBCK_16			0x00
#define REG_PSBCK_32			0x10
#define REG_PSBCK_64			0x20
#define REG_PSBCK_128			0x30
#define REG_PSBCK_256			0x40
#define REG_PSLRCK_0_25fs		0x00
#define REG_PSLRCK_0_5fs		0x01
#define REG_PSLRCK_1fs			0x02
#define REG_PSLRCK_2fs			0x03
#define REG_PSLRCK_4fs			0x04
// 34h
#define REG_RX0DIS				0x80
#define REG_RX1DIS				0x40
#define REG_RXSEL_RXIN0   		0x00
#define REG_RXSEL_RXIN1   		0x01
#define REG_RXSEL_RXIN2   		0x02		//default
#define REG_RXSEL_RXIN3   		0x03
// 37h
#define REG_PFSTGT_DIR			0x00
#define REG_PFSTGT_ADC			0x01
#define REG_PFSTGT_AUXIN0		0x02
#define REG_PFSTGT_AUXIN1		0x03
#define REG_PFSTGT_AUXIN2		0x04
#define REG_PFSTGT_MAIN_OUTPUT	0x05
#define REG_PFSTGT_AUX_PORT		0x06
#define REG_PFSTGT_DIT			0x07
// 40h
#define REG_MRST				0x80
#define REG_SRST				0x40
#define REG_ADDIS				0x20
#define REG_RXDIS				0x10
#define REG_TXDIS				0x02
#define REG_XODIS				0x01
// 60h
#define REG_TXSSRC_AUTO			0x00
#define REG_TXSSRC_DIR			0x10
#define REG_TXSSRC_ADC			0x20
#define REG_TXSSRC_AUXIN0		0x30
#define REG_TXSSRC_AUXIN1		0x40
#define REG_TXSSRC_AUXIN2		0x50
#define REG_TXPSRC_AUTO			0x00
#define REG_TXPSRC_DIR			0x01
#define REG_TXPSRC_ADC			0x02
#define REG_TXPSRC_AUXIN0		0x03
#define REG_TXPSRC_AUXIN1		0x04
#define REG_TXPSRC_AUXIN2		0x05
// 61h
#define REG_TXSCK_128			0x00
#define REG_TXSCK_256			0x10
#define REG_TXSCK_512			0x20
#define REG_TXSCK_DIR			0x70
#define REG_TXDSD				0x04
#define REG_TXFMT_i2s			0x00
#define REG_TXFMT_24_LEFT		0x01
#define REG_TXFMT_24_RIGHT		0x02
#define REG_TXFMT_16_RIGHT		0x03
// 6Bh
#define REG_MOSSRC_AUTO			0x00
#define REG_MOSSRC_DIR			0x10
#define REG_MOSSRC_ADC			0x20
#define REG_MOSSRC_AUXIN0		0x30
#define REG_MOSSRC_AUXIN1		0x40
#define REG_MOSSRC_AUXIN2		0x50
#define REG_MOPSRC_AUTO			0x00
#define REG_MOPSRC_DIR			0x01
#define REG_MOPSRC_ADC			0x02
#define REG_MOPSRC_AUXIN0		0x03
#define REG_MOPSRC_AUXIN1		0x04
#define REG_MOPSRC_AUXIN2		0x05
// 78h
#define REG_MPO0SEL_HIZ			0x00
#define REG_MPO0SEL_HI			0x01
#define REG_MPO0SEL_LOW			0x02
#define REG_MPO0SEL_VOUT		0x03
#define REG_MPO0SEL_INT0		0x04
#define REG_MPO0SEL_INT1		0x05
#define REG_MPO0SEL_CLKST		0x06
#define REG_MPO0SEL_EMPH		0x07
#define REG_MPO0SEL_XMCKO		0x0C
#define REG_MPO0SEL_TXOUT		0x0D
#define REG_MPO0SEL_RECOUT0		0x0E
#define REG_MPO0SEL_RECOUT1		0x0F
//--------------------------------
#define REG_MPO1SEL_HIZ			0x00
#define REG_MPO1SEL_HI			0x10
#define REG_MPO1SEL_LOW			0x20
#define REG_MPO1SEL_VOUT		0x30
#define REG_MPO1SEL_INT0		0x40
#define REG_MPO1SEL_INT1		0x50
#define REG_MPO1SEL_CLKST		0x60
#define REG_MPO1SEL_EMPH		0x70
#define REG_MPO1SEL_XMCKO		0xC0
#define REG_MPO1SEL_TXOUT		0xD0
#define REG_MPO1SEL_RECOUT0		0xE0
#define REG_MPO1SEL_RECOUT1		0xF0


/* Exported macro ------------------------------------------------------------*/
//	DIR input sources macros

// S/PDIF Channel status buffer registers
typedef struct
{
	uint8_t	REG_A;
	uint8_t	REG_B;
	uint8_t	REG_C;
	uint8_t	REG_D;
	uint8_t	REG_E;
	uint8_t	REG_F;
}CHANNEL_BUFFERS_t;

typedef struct
{
	uint8_t port_38;		// register 38
	uint8_t freq_38;		// register 38
	uint8_t audio_39;		// register 39
	uint8_t freq_39;		// register 39
}SAMPLING_FREQ_INFO_t;


//	PCM mode enum
typedef enum
{
	PCM_TX = 1,
	PCM_RX,
	PCM_SS_SELECT,
	PCM_SS_DESELECT
}PCM9211_Mode_t;

typedef struct
{
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
	PCM9211(SPI_HandleTypeDef* spi, PCM9211_Mode_t mode, SS_Conf* SS, CHANNEL_BUFFERS_t* buffers);
	virtual ~PCM9211();

//	uint8_t PCM9211_Init(I2C_HandleTypeDef *i2c, uint8_t address);
	uint8_t PCM9211_Init(uint8_t state);//SPI_HandleTypeDef *spi, PCM9211_Mode_t mode);
	uint8_t Read_(uint8_t reg);
	void Write_(uint8_t reg, uint8_t data);
	void Write_regMask(uint8_t reg, uint8_t data);
	uint32_t PCM9211_DataInfromation();
	uint8_t Channel_Buffers(void);

	SAMPLING_FREQ_INFO_t PCM9211_SamplingFreq(void);
private:
	uint8_t Write_REG(uint8_t reg, uint8_t data);
	void PCM9211_SelectSource(uint8_t source);
	void PCM9211_SS(PCM9211_Mode_t state);


	SPI_HandleTypeDef* _spi;
	PCM9211_Mode_t _mode;
	GPIO_TypeDef* _SS_port;
	uint16_t _SS_pin;
	bool _isReset;

	CHANNEL_BUFFERS_t* _buffers;
};

#endif /* PCM9211_INC_PCM9211_HPP_ */

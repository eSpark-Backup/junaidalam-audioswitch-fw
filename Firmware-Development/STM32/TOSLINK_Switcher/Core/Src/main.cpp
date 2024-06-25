/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
/ USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <PCM9211.hpp>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

CHANNEL_BUFFERS_t channel_buffers;				// to hold S/PDIF channel information
CHANNEL_BUFFERS_t channel_buffers_tx =
{
		.REG_A = 0x00,					// 0b00000000
		.REG_B = 0xB8,					// 0b						// receiving 0xb8(0b10111000) from RX
		.REG_C = 0x00,
		.REG_D = 0x00,					// 0b00000000
		.REG_E = 0x00,
		.REG_F = 0x00
};

typedef struct
{
	uint16_t output_pin;				// have to figure this out
	uint8_t board_volume;
}output_routes_t;

output_routes_t opt4 = {
		.output_pin = SW_S0_Pin,
		.board_volume = 50
};
output_routes_t opt3 = {
		.output_pin = SW_S1_Pin,
		.board_volume = 50
};
output_routes_t opt2 = {
		.output_pin = SW_S2_Pin,
		.board_volume = 50
};
output_routes_t opt1 = {
		.output_pin = SW_S2_Pin | SW_S1_Pin,
		.board_volume = 50
};

struct current_route_s
{
	output_routes_t* route;
	uint8_t state;
};

typedef enum STATES
{
	ENABLE_OPT = 1,
	DISABLE_OPT,
	VOLUME_INCREASE,
	VOLUME_DECREASE
}STATES_t;

current_route_s current_route = { NULL, DISABLE_OPT };

SS_Conf tx_conf = {
	.port = SS_T_GPIO_Port,
	.pin = SS_T_Pin							// SPI SS pin
};
SS_Conf rx_conf = {
	.port = SS_R_GPIO_Port,
	.pin = SS_R_Pin							// SPI SS pin
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUM_FRAMES 256//128							// No. of frames per DMA transaction, we are using I2S format with 24bit width in 32bit frame
#define BUFFER_SIZE (NUM_FRAMES * sizeof(uint32_t))
#define VOLUME_LEVELS	20.f					//resolution of volume, 0-100
#define VOLUME_STEP_SIZE	(1.f/VOLUME_LEVELS)*100.f
#define SWITCH_VOLUME(x)	(x)/(100.f)	// switch range from 0-100 to 0-1
#define OPT1	0x3			// 0b0011
#define OPT2	0x1			// 0b0001
#define OPT3	0x2			// 0b0010
#define OPT4	0x4			// 0b0100
#define OPT_DISABLE		0x8	// 0b1000			//  ( EN | S0 | S1 | S2)
#define ERROR_READ_RX_PCM	HAL_GPIO_ReadPin(ERROR_INT0_R_GPIO_Port, ERROR_INT0_R_Pin)

#define DEBOUNCE_TIME_MS	250					// Define the debounce time in milliseconds
#define OPT_TURNOFF_TIME	30					// Auto Turnoff time for output; in Seconds

uint32_t rxBuffer[NUM_FRAMES];
//uint32_t txBuffer[NUM_FRAMES];

uint32_t txBuffer[NUM_FRAMES];
//uint32_t txBuffer[NUM_FRAMES] = {									//	1KHz Sine wave at 44.1KHz sampling rate
//16383,  18709,  20989,  23174,  25223,  27092,  28744,  30146,
//31268,  32089,  32592,  32766,  32609,  32122,  31317,  30209,
//28820,  27180,  25321,  23281,  21100,  18825,  16500,  14172,
//11890,   9698,   7642,   5763,   4099,   2684,   1547,    710,
//191,      0,    142,    612,   1402,   2495,   3870,   5499,
//7348,   9380,  11554,  13826,  16150,  18478,  20764,  22961,
//25025,  26914,  28590,  30018,  31169,  32021,  32556,  32764,
//32639,  32185,  31411,  30332,  28971,  27354,  25515,  23492,
//21324,  19055,  16733,  14404,  12114,   9912,   7840,   5942,
//4255,   2814,   1647,    780,    229,      5,    113,    550,
//1309,   2373,   3721,   5325,   7154,   9169,  11331,  13596,
//15916,  18246,  20539,  22747,  24826,  26734,  28433,  29887,
//31067,  31950,  32518,  32758,  32667,  32245,  31503,  30453,
//29119,  27527,  25708,  23701,  21546,  19285,  16966,  14636,
//12340,  10127,   8040,   6122,   4413,   2946,   1751,    853,
//269,     12,     87,    492,   1219,   2253,   3574,   5154,
//6962,   8961,  11110,  13366,  15683,  18014,  20313,  22531,
//24625,  26552,  28273,  29753,  30962,  31876,  32475,  32749,
//32691,  32302,  31591,  30572,  29265,  27697,  25899,  23909,
//21767,  19515,  17200,  14868,  12567,  10343,   8242,   6306,
//4573,   3081,   1857,    929,    313,     23,     64,    437,
//1132,   2136,   3430,   4985,   6772,   8753,  10889,  13137,
//15450,  17782,  20086,  22314,  24423,  26368,  28111,  29617,
//30854,  31798,  32430,  32736,  32712,  32356,  31676,  30687,
//29408,  27864,  26088,  24116,  21986,  19744,  17433,  15100,
//12794,  10561,   8445,   6491,   4736,   3218,   1967,   1008,
//360,     54,     30,    336,   1048,   2023,   3288,   4819,
//6584,   8548,  10670,  12908,  15217,  17549,  19858,  22096,
//24218,  26182,  27947,  29478,  30743,  31718,  32381,  32721,
//32729,  32406,  31758,  30799,  29548,  28030,  26275,  24321,
//22205,  19972,  17666,  15333,  13022,  10780,   8650,   6678,
//4902,   3358,   2079,   1090,    410,     54,     30,    336
//};

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2S_HandleTypeDef hi2s2;
I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_spi2_rx;										// I2S receive SPI
DMA_HandleTypeDef hdma_spi3_tx;										// I2S transmit SPI

SPI_HandleTypeDef hspi1;											// Communication SPI with PCM ICs
TIM_HandleTypeDef htim11;											// Timer for OPT auto turnoff feature


/* USER CODE BEGIN PV */
PCM9211 pcmRX(&hspi1, PCM_RX, &rx_conf, &channel_buffers);				// Receiving PCM IC Object
PCM9211 pcmTX(&hspi1, PCM_TX, &tx_conf, &channel_buffers_tx);			// Transmitting PCM IC Object

volatile uint8_t RXdma_started = 0;							// disables false RxHalfCpltCallback at startup
volatile uint8_t RXPCM_ERROR_flag = DISABLE_OPT;			// flag for activating TX PCM IC
static	uint32_t last_interrupt_time = 0;					// Variable to manage the debounce logic
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2S2_Init(void);
static void MX_I2S3_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM11_Init(void);
/* USER CODE BEGIN PFP */
void volume_control(STATES_t state);
void route_Control(GPIO_TypeDef* GPIOx, uint8_t config);
void Timer_Control(STATES_t state);
void Reset_Timer(TIM_HandleTypeDef *htim);					// not utilised
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI2)
    {
        // Process first half of the buffer
    	if(RXdma_started){
			for (int i = 0; i < NUM_FRAMES / 2; i++)
			{
//				int32_t sample = rxBuffer[i];
//				sample = (sample >> 8) * SWITCH_VOLUME(current_route.route->board_volume); // Adjust volume and shift to 24-bit
//
//				if (sample > 0x7FFFFF) sample = 0x7FFFFF;
//				if (sample < -0x800000) sample = -0x800000;

//				txBuffer[i] = sample << 8; // Shift back to 32-bit format
//            txBuffer[i] = rxBuffer[i];
			}

			// Transmit the buffer
//			if (HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)txBuffer, NUM_FRAMES / 2) != HAL_OK)
//			{
//			  // Initialization Error
//			  Error_Handler();
//			}
    	}
    	else
    	{
    		RXdma_started = 1;
    	}
    }
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI2)
    {
        // Process second half of the buffer
        for (int i = NUM_FRAMES / 2; i < NUM_FRAMES ; i++)
        {
//            int32_t sample = rxBuffer[i];
//            sample = (sample >> 8) * SWITCH_VOLUME(current_route.route->board_volume); // Adjust volume and shift to 24-bit
//
//            if (sample > 0x7FFFFF) sample = 0x7FFFFF;
//            if (sample < -0x800000) sample = -0x800000;

//            txBuffer[i] = sample << 8; // Shift back to 32-bit format
//            txBuffer[i] = rxBuffer[i];
        }

        // Transmit the buffer
//		if (HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)txBuffer, NUM_FRAMES / 2) != HAL_OK)
//		{
//		  // Initialization Error
//		  Error_Handler();
//		}
    }
}

//void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
//{
//    if (hi2s->Instance == SPI2)
//    {
//        // Process second half of the buffer
//        for (int i = 0; i < NUM_FRAMES; i++)
//        {
//            int32_t sample = rxBuffer[i];
//            sample = (sample >> 8) * SWITCH_VOLUME(current_route.route->board_volume); // Adjust volume and shift to 24-bit
//
//            if (sample > 0x7FFFFF) sample = 0x7FFFFF;
//            if (sample < -0x800000) sample = -0x800000;
//
//            txBuffer[i] = sample << 8; // Shift back to 32-bit format
//        }
//    }
//}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI3)
    {
        // Transmission complete, ready for next transfer if needed
        // For continuous transfer, you might reinitialize or handle buffer swapping here
    }
}

// HAL TIM Callback function
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM11)
    {
    	Timer_Control(DISABLE_OPT);
    }
}

// HAL GPIO EXTI Callback function
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	uint32_t current_time = HAL_GetTick(); // Get current time in milliseconds

	// Check if the debounce period has passed
	if ((current_time - last_interrupt_time) >= DEBOUNCE_TIME_MS)
	{
		// SW_Select buttons
		switch(GPIO_Pin)
		{
		case SW1_S_Pin:
			if(current_route.state == DISABLE_OPT || current_route.route != &opt1)
			{
				route_Control(SW_E_GPIO_Port, OPT1);
				Timer_Control(ENABLE_OPT);
				current_route.route = &opt1;
			}
			else
			{
				Timer_Control(DISABLE_OPT);
			}
			break;
		case SW2_S_Pin:
			if(current_route.state == DISABLE_OPT || current_route.route != &opt2)
			{
				route_Control(SW_E_GPIO_Port, OPT2);
				Timer_Control(ENABLE_OPT);
				current_route.route = &opt2;
			}
			else
			{
				Timer_Control(DISABLE_OPT);
			}
			break;
		case SW3_S_Pin:
			if(current_route.state == DISABLE_OPT || current_route.route != &opt3)
			{
				route_Control(SW_E_GPIO_Port, OPT3);
				Timer_Control(ENABLE_OPT);
				current_route.route = &opt3;
			}
			else
			{
				Timer_Control(DISABLE_OPT);
			}
			break;
		case SW4_S_Pin:
			if(current_route.state == DISABLE_OPT || current_route.route != &opt4)
			{
				route_Control(SW_E_GPIO_Port, OPT4);
				Timer_Control(ENABLE_OPT);
				current_route.route = &opt4;
			}
			else
			{
				Timer_Control(DISABLE_OPT);
			}
			break;
		default:
			break;
		}

		// Volume control
		if (GPIO_Pin == SW1_V__Pin && current_route.route == &opt1)
		{
			volume_control(VOLUME_INCREASE);
		}
		if (GPIO_Pin == SW1_V_B7_Pin && current_route.route == &opt1)
		{
			volume_control(VOLUME_DECREASE);
		}

		if (GPIO_Pin == SW2_V__Pin && current_route.route == &opt2)
		{
			volume_control(VOLUME_INCREASE);
		}
		if (GPIO_Pin == SW2_V_C15_Pin && current_route.route == &opt2)
		{
			volume_control(VOLUME_DECREASE);
		}

		if (GPIO_Pin == SW3_V__Pin && current_route.route == &opt3)
		{
			volume_control(VOLUME_INCREASE);
		}
		if (GPIO_Pin == SW3_V_A2_Pin && current_route.route == &opt3)
		{
			volume_control(VOLUME_DECREASE);
		}

		if (GPIO_Pin == SW4_V__Pin && current_route.route == &opt4)
		{
			volume_control(VOLUME_INCREASE);
		}
		if (GPIO_Pin == SW4_V_C12_Pin && current_route.route == &opt4)
		{
			volume_control(VOLUME_DECREASE);
		}

		last_interrupt_time = current_time;
	}
}


// Function to control the S0|S1|S2|EN pins on 74LV4051PW Multiplexing IC
void route_Control(GPIO_TypeDef* GPIOx, uint8_t config)
{
  // Clear all the relevant pins first
  GPIOx->BSRR = ((SW_E_Pin | SW_S0_Pin | SW_S1_Pin | SW_S2_Pin) << 16); // Reset relevant pins

  // Apply the new configuration
  if (config & 0x4)
  {
	  GPIOx->BSRR = (uint32_t)SW_S0_Pin; // Set
  }
  if (config & 0x2)
  {
	  GPIOx->BSRR = (uint32_t)SW_S1_Pin; // Set
  }
  if (config & 0x1)
  {
	  GPIOx->BSRR = (uint32_t)SW_S2_Pin; // Set
  }
  if (config & 0x8)
  {
	  GPIOx->BSRR = (uint32_t)SW_E_Pin; // Set
  }
}

void volume_control(STATES_t state)
{
	switch(state)
	{
	case VOLUME_INCREASE:
		if(current_route.route->board_volume < 100)
		{
			current_route.route->board_volume += VOLUME_STEP_SIZE;
		}
		if(current_route.route->board_volume > 100)
			current_route.route->board_volume = 100;
		break;
	case VOLUME_DECREASE:
		if(current_route.route->board_volume > 0)
		{
			current_route.route->board_volume -= VOLUME_STEP_SIZE;
		}
		if(current_route.route->board_volume < 0)
			current_route.route->board_volume = 0;
		break;
	default:
		break;
	}
}

void Timer_Control(STATES_t state)
{
	switch(state)
	{
	case ENABLE_OPT:
		current_route.state = ENABLE_OPT;
		Reset_Timer(&htim11);
		// Stop the timer
		if(htim11.State != HAL_TIM_STATE_READY)
		{
			if (HAL_TIM_Base_Stop_IT(&htim11) != HAL_OK)
			{
				// Stopping Error
				Error_Handler();
			}
		}
		// Start the timer
		if (HAL_TIM_Base_Start_IT(&htim11) != HAL_OK)
		{
			// Starting Error
			Error_Handler();
		}

		break;
	case DISABLE_OPT:
		route_Control(SW_E_GPIO_Port, OPT_DISABLE);
		current_route.state = DISABLE_OPT;
		current_route.route = NULL;

		// Stop the timer
		if (HAL_TIM_Base_Stop_IT(&htim11) != HAL_OK)
		{
			// Stopping Error
			Error_Handler();
		}
		break;
	default:
		break;
	}
}

// Function to reset the timer countdown
void Reset_Timer(TIM_HandleTypeDef *htim)
{
    __HAL_TIM_SET_COUNTER(htim, 0);
}


	uint8_t readvalueREG;
	SAMPLING_FREQ_INFO_t Sampling_Frequency_RX;
	SAMPLING_FREQ_INFO_t Sampling_Frequency_TX;

	GPIO_PinState error_int0;
	GPIO_PinState npcm_int1;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2S2_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */
  PCM9211_Reset();


  int index = 0;
  while(index<NUM_FRAMES)//for(int index=0; index<NUM_FRAMES; index+2)
  {
	  txBuffer[index] = 3333;
	  index+=2;
  }
  index=1;
  while(index<NUM_FRAMES)//for(int index=1; index<NUM_FRAMES; index+2)
  {
	  txBuffer[index] = 1515;
	  index+=2;
  }



  // Initialize PCM Receiver ICs
  if(pcmRX.PCM9211_Init(0))
  {
	  // Initialization Error
	  Error_Handler();
  }
  // Initialize PCM Transmitter ICs
  if(pcmTX.PCM9211_Init(0))
  {
	  // Initialization Error
	  Error_Handler();
  }
  pcmTX.Channel_Buffers_Set();


  if (HAL_I2S_Receive_DMA(&hi2s2, (uint16_t*)rxBuffer, NUM_FRAMES/2) != HAL_OK)
  {
	  // Initialization Error
	  Error_Handler();
  }

  // Start I2S3 in DMA mode (transmit)
  if (HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)txBuffer, NUM_FRAMES/2) != HAL_OK)
  {
	  // Initialization Error
	  Error_Handler();
  }

//	readvalueREG = pcmRX.Read_(0x40);
//	readvalueREG = pcmRX.Read_(0x20);
//	readvalueREG = pcmRX.Read_(0x24);
//	readvalueREG = pcmRX.Read_(0x37);
//	readvalueREG = pcmRX.Read_(0x60);
//	readvalueREG = pcmRX.Read_(0x61);
//	readvalueREG = pcmRX.Read_(0x78);
//	readvalueREG = pcmRX.Read_(0x62);
//
//	readvalueREG = pcmTX.Read_(0x40);
//	readvalueREG = pcmTX.Read_(0x20);
//	readvalueREG = pcmTX.Read_(0x24);
//	readvalueREG = pcmTX.Read_(0x37);
//	readvalueREG = pcmTX.Read_(0x60);
//	readvalueREG = pcmTX.Read_(0x61);
//	readvalueREG = pcmTX.Read_(0x78);
//	readvalueREG = pcmTX.Read_(0x62);


	current_route.route = &opt2;
	current_route.state = ENABLE_OPT;
	route_Control(SW_E_GPIO_Port, OPT2);

  /* USER CODE END 2 */


  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


//	  if(!ERROR_READ_RX_PCM && (RXPCM_ERROR_flag == DISABLE_OPT))
//	  {
//		  // Start I2S2 in DMA mode (receive)
//		  pcmTX.PCM9211_Init(0);
//
//		  pcmRX.Channel_Buffers_Get();
//		  pcmTX.Channel_Buffers_Set();
//
//		  RXPCM_ERROR_flag = ENABLE_OPT;
//
//		  if (HAL_I2S_Receive_DMA(&hi2s2, (uint16_t*)rxBuffer, NUM_FRAMES/2) != HAL_OK)
//		  {
//			  // Initialization Error
//			  Error_Handler();
//		  }
//
//		  // Start I2S3 in DMA mode (transmit)
//		  if (HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)rxBuffer, NUM_FRAMES/2) != HAL_OK)
//		  {
//			  // Initialization Error
//			  Error_Handler();
//		  }
//	  }
//	  else if(ERROR_READ_RX_PCM && (RXPCM_ERROR_flag == ENABLE_OPT))
//	  {
////		if (HAL_I2S_DMAStop(&hi2s2) != HAL_OK)
////		{
////			Error_Handler();
////		}
//		if (HAL_I2S_DMAStop(&hi2s3) != HAL_OK)
//		{
//			Error_Handler();
//		}
//		pcmTX.PCM9211_Init(1);
//		pcmRX.PCM9211_Init(0);			// check authenticity of this
//		RXPCM_ERROR_flag = DISABLE_OPT;
//	  }



	  npcm_int1 = HAL_GPIO_ReadPin(NPCM_INT1_R_GPIO_Port, NPCM_INT1_R_Pin);
	  error_int0 = ERROR_READ_RX_PCM;
	  if(!error_int0)
	  {
		  pcmRX.Channel_Buffers_Get();
	  }

	  Sampling_Frequency_RX = pcmRX.PCM9211_SamplingFreq();
	  Sampling_Frequency_TX = pcmTX.PCM9211_SamplingFreq();

	  HAL_GPIO_TogglePin(COP_GPIO_Port, COP_Pin);
	  HAL_Delay(500);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
	  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	  /** Configure the main internal regulator output voltage
	  */
	  __HAL_RCC_PWR_CLK_ENABLE();
	  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	  /** Initializes the RCC Oscillators according to the specified parameters
	  * in the RCC_OscInitTypeDef structure.
	  */
	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	  RCC_OscInitStruct.PLL.PLLM = 2;
	  RCC_OscInitStruct.PLL.PLLN = 168;
	  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV6;
	  RCC_OscInitStruct.PLL.PLLQ = 4;
	  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	  {
	    Error_Handler();
	  }

	  /** Initializes the CPU, AHB and APB buses clocks
	  */
	  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV4;
	  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	/** Initializes the peripherals clock
	*/
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
	PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
	PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
  * @brief I2S2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S2_Init(void)
{

  /* USER CODE BEGIN I2S2_Init 0 */

  /* USER CODE END I2S2_Init 0 */

  /* USER CODE BEGIN I2S2_Init 1 */

  /* USER CODE END I2S2_Init 1 */
  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_RX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_24B;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_44K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S2_Init 2 */

  /* USER CODE END I2S2_Init 2 */

}

/**
  * @brief I2S3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S3_Init(void)
{

  /* USER CODE BEGIN I2S3_Init 0 */

  /* USER CODE END I2S3_Init 0 */

  /* USER CODE BEGIN I2S3_Init 1 */

  /* USER CODE END I2S3_Init 1 */
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_24B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE; //I2S_MCLKOUTPUT_DISABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_44K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S3_Init 2 */

  /* USER CODE END I2S3_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */
  __HAL_RCC_TIM11_CLK_ENABLE();

  uint32_t timer_clock_frequency = HAL_RCC_GetPCLK2Freq();
  uint32_t prescaler_value = (timer_clock_frequency / 1000) - 1;		// 1ms timer increments
  uint32_t period_value = ((OPT_TURNOFF_TIME * timer_clock_frequency)/(prescaler_value+1)) - 1;
  /* USER CODE END TIM11_Init 0 */

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = prescaler_value; //9999;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = period_value; //41999;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }

//  if (HAL_TIM_OnePulse_Init(&htim11, TIM_OPMODE_SINGLE) != HAL_OK)
//  {
//    Error_Handler();
//  }

  /* USER CODE BEGIN TIM11_Init 2 */
  // Enable the TIM11 global interrupt
  TIM_Base_SetConfig(htim11.Instance, &htim11.Init);
  HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
  /* USER CODE END TIM11_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
//  __HAL_RCC_DMA2_CLK_ENABLE();


  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
  /* DMA1_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin
//                          |LED8_Pin|SS_R_Pin|RESET_Pin|LED10_Pin
                          |LED8_Pin|SS_R_Pin|RESET_Pin
                          |LED11_Pin|LED12_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED9_Pin|SS_T_Pin|LED2_Pin|SW_E_Pin
                          |SW_S0_Pin|SW_S1_Pin|SW_S2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED3_Pin|COP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SS_T_GPIO_Port, SS_T_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SS_R_GPIO_Port, SS_R_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : SW2_S_Pin SW2_V__Pin SW2_V_C15_Pin SW4_S_Pin
                           SW4_V__Pin SW4_V_C12_Pin */
  GPIO_InitStruct.Pin = SW2_S_Pin|SW2_V__Pin|SW2_V_C15_Pin|SW4_S_Pin
                          |SW4_V__Pin|SW4_V_C12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED4_Pin LED5_Pin LED6_Pin LED7_Pin
                           LED8_Pin SS_R_Pin RESET_Pin LED10_Pin
                           LED11_Pin LED12_Pin */
  GPIO_InitStruct.Pin = LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin
//                          |LED8_Pin|SS_R_Pin|RESET_Pin|LED10_Pin
                          |LED8_Pin|SS_R_Pin|RESET_Pin
                          |LED11_Pin|LED12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : SW3_S_Pin SW3_V__Pin SW3_V_A2_Pin */
  GPIO_InitStruct.Pin = SW3_S_Pin|SW3_V__Pin|SW3_V_A2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LED9_Pin SS_T_Pin LED2_Pin SW_E_Pin
                           SW_S0_Pin SW_S1_Pin SW_S2_Pin */
  GPIO_InitStruct.Pin = LED9_Pin|SS_T_Pin|LED2_Pin|SW_E_Pin
                          |SW_S0_Pin|SW_S1_Pin|SW_S2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : NPCM_INT1_T_Pin ERROR_INT0_T_Pin NPCM_INT1_R_Pin ERROR_INT0_R_Pin */
  GPIO_InitStruct.Pin = NPCM_INT1_T_Pin|ERROR_INT0_T_Pin|NPCM_INT1_R_Pin|ERROR_INT0_R_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LED1_Pin */
  GPIO_InitStruct.Pin = LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SW1_S_Pin SW1_V__Pin SW1_V_B7_Pin */
  GPIO_InitStruct.Pin = SW1_S_Pin|SW1_V__Pin|SW1_V_B7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LED3_Pin COP_Pin */
  GPIO_InitStruct.Pin = LED3_Pin|COP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
//  HAL_GPIO_WritePin(GPIOC, LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin|LED8_Pin|LED10_Pin
  HAL_GPIO_WritePin(GPIOC, LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin|LED8_Pin
                          |LED11_Pin|LED12_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED9_Pin|LED2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED3_Pin, GPIO_PIN_SET);

  /* EXTI interrupt init*/
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
//  printf("Error was encountered from %p\r\n", __builtin_return_address(0));
  while (1)
  {
//	  HAL_GPIO_TogglePin(COP_GPIO_Port, COP_Pin);
//	  HAL_Delay(500);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
     printf("Wrong parameters value: file %s on line %ld\r\n", file, line);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

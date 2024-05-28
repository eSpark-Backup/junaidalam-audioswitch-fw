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
#include <PCM9211.hpp>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct{
	uint16_t output_pin;
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

struct{
	output_routes_t* route;
	uint8_t state;
}current_route;

typedef enum STATES{
	ENABLE_OPT = 1,
	DISABLE_OPT,
	VOLUME_INCREASE,
	VOLUME_DECREASE
}STATES_T;

SS_Conf tx_conf = {
	.port = SS_T_GPIO_Port,
	.pin = SS_T_Pin
};
SS_Conf rx_conf = {
	.port = SS_R_GPIO_Port,
	.pin = SS_R_Pin
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUM_FRAMES 128							// No. of frames per DMA transaction, we are using I2S format with 24bit width in 32bit frame
#define BUFFER_SIZE (NUM_FRAMES * sizeof(uint32_t))
#define VOLUME_LEVELS	100						//resolution of volume, 0-100
#define VOLUME_STEP_SIZE	(1.0/VOLUME_LEVELS)*100.0
#define SWITCH_VOLUME(x)	(x)/VOLUME_LEVELS
#define OPT4	0x4			// 0b0100			//  ( EN | S0 | S1 | S2)
#define OPT3	0x2			// 0b0010
#define OPT2	0x1			// 0b0001
#define OPT1	0x3			// 0b0011
#define OPT_DISABLE		0x8	// 0b1000

uint32_t rxBuffer[NUM_FRAMES];
uint32_t txBuffer[NUM_FRAMES];
//float volume = 0.5; // Volume factor (0.0 to 1.0), default value is 0.5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2S_HandleTypeDef hi2s2;
I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_spi2_tx;
DMA_HandleTypeDef hdma_spi3_rx;

SPI_HandleTypeDef hspi1;
TIM_HandleTypeDef htim11;

PCM9211 pcmTX(&hspi1, PCM_TX, &tx_conf);
PCM9211 pcmRX(&hspi1, PCM_RX, &rx_conf);

/* USER CODE BEGIN PV */

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
void volume_control(STATES_T state);
void route_Control(GPIO_TypeDef* GPIOx, uint8_t config);
void Timer_Control(STATES_T state);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI3)
    {
        // Process first half of the buffer
        for (int i = 0; i < NUM_FRAMES / 2; i++)
        {
            int32_t sample = rxBuffer[i];
            sample = (sample >> 8) * (float)SWITCH_VOLUME(current_route.route->board_volume); // Adjust volume and shift to 24-bit

            if (sample > 0x7FFFFF) sample = 0x7FFFFF;
            if (sample < -0x800000) sample = -0x800000;

            txBuffer[i] = sample << 8; // Shift back to 32-bit format
        }
    }
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI3)
    {
        // Process second half of the buffer
        for (int i = NUM_FRAMES / 2; i < NUM_FRAMES; i++)
        {
            int32_t sample = rxBuffer[i];
            sample = (sample >> 8) * (float)SWITCH_VOLUME(current_route.route->board_volume); // Adjust volume and shift to 24-bit

            if (sample > 0x7FFFFF) sample = 0x7FFFFF;
            if (sample < -0x800000) sample = -0x800000;

            txBuffer[i] = sample << 8; // Shift back to 32-bit format
        }
    }
}

//void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
//{
//    if (hi2s->Instance == SPI2)
//    {
//        // Transmission complete, ready for next transfer if needed
//        // For continuous transfer, you might reinitialize or handle buffer swapping here
//    }
//}

// HAL GPIO EXTI Callback function
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// SW_Select buttons
    if (GPIO_Pin == SW1_S_Pin)
    {
    	current_route.route = &opt1;
    	if(current_route.state == 0){
    		route_Control(SW_E_GPIO_Port, OPT1);
    		Timer_Control(ENABLE_OPT);
    	}
    	else{
    		Timer_Control(DISABLE_OPT);
    	}
    }
    if (GPIO_Pin == SW2_S_Pin)
    {
    	current_route.route = &opt2;
    	if(current_route.state == 0){
    		route_Control(SW_E_GPIO_Port, OPT2);
    		Timer_Control(ENABLE_OPT);
    	}
    	else{
    		Timer_Control(DISABLE_OPT);
    	}
    }
    if (GPIO_Pin == SW3_S_Pin)
    {
    	current_route.route = &opt3;
    	if(current_route.state == 0){
    		route_Control(SW_E_GPIO_Port, OPT3);
    		Timer_Control(ENABLE_OPT);
    	}
    	else{
    		Timer_Control(DISABLE_OPT);
    	}
    }
    if (GPIO_Pin == SW4_S_Pin)
    {
    	current_route.route = &opt4;
    	if(current_route.state == 0){
    		route_Control(SW_E_GPIO_Port, OPT4);
    		Timer_Control(ENABLE_OPT);
    	}
    	else{
    		Timer_Control(DISABLE_OPT);
    	}
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
}


// Function to control the S0|S1|S2|EN pins on 74LV4051PW Multiplexing IC
void route_Control(GPIO_TypeDef* GPIOx, uint8_t config)
{
  // Clear all the relevant pins first
  GPIOx->BSRR = ((SW_E_Pin | SW_S0_Pin | SW_S1_Pin | SW_S2_Pin) << 16); // Reset relevant pins

  // Apply the new configuration
  if (config & 0x4) {
	  GPIOx->BSRR = (uint32_t)SW_S0_Pin; // Set
  }
  if (config & 0x2) {
	  GPIOx->BSRR = (uint32_t)SW_S1_Pin; // Set
  }
  if (config & 0x1) {
	  GPIOx->BSRR = (uint32_t)SW_S2_Pin; // Set
  }
  if (config & 0x8) {
	  GPIOx->BSRR = (uint32_t)SW_E_Pin; // Set
  }
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM11) {
    	Timer_Control(DISABLE_OPT);
    }
}

void volume_control(STATES_T state){
	switch(state){
	case VOLUME_INCREASE:
		if(current_route.route->board_volume < 100){
			current_route.route->board_volume += VOLUME_STEP_SIZE;
		}
		if(current_route.route->board_volume > 100)
			current_route.route->board_volume = 100;
		break;
	case VOLUME_DECREASE:
		if(current_route.route->board_volume > 0){
			current_route.route->board_volume -= VOLUME_STEP_SIZE;
		}
		if(current_route.route->board_volume < 0)
			current_route.route->board_volume = 0;
		break;
	default:
		break;
	}
}

void Timer_Control(STATES_T state){
	switch(state){
	case DISABLE_OPT:
		route_Control(SW_E_GPIO_Port, OPT_DISABLE);
		current_route.state = 0;

		// Stop the timer
		if (HAL_TIM_Base_Stop_IT(&htim11) != HAL_OK) {
			// Stopping Error
			Error_Handler();
		}
		break;
	case ENABLE_OPT:
		current_route.state = 1;
		// Start the timer
		if (HAL_TIM_Base_Start_IT(&htim11) != HAL_OK) {
			// Starting Error
			Error_Handler();
		}
		break;
	default:
		break;
	}
}
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

  // Initialize PCM ICs
  if(pcmRX.PCM9211_Init())
  {
	  // Initialization Error
	  Error_Handler();
  }
  if(pcmTX.PCM9211_Init())
  {
	  // Initialization Error
	  Error_Handler();
  }

  // Start I2S2 in DMA mode (receive)
  if (HAL_I2S_Receive_DMA(&hi2s3, (uint16_t*)rxBuffer, NUM_FRAMES) != HAL_OK)
  {
	  // Initialization Error
	  Error_Handler();
  }

  // Start I2S3 in DMA mode (transmit)
  if (HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)txBuffer, NUM_FRAMES) != HAL_OK)
  {
	  // Initialization Error
	  Error_Handler();
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Macro to configure the PLL multiplication factor
  */
  __HAL_RCC_PLL_PLLM_CONFIG(16);

  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSI);

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
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
  hi2s2.Init.Mode = I2S_MODE_MASTER_TX;
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
  hi2s3.Init.Mode = I2S_MODE_MASTER_RX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_24B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
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
  hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
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

  /* USER CODE END TIM11_Init 0 */

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 15999;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 30000-1;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  // Configure DMA for I2S2_RX
  hdma_spi3_rx.Instance = DMA1_Stream3;
  hdma_spi3_rx.Init.Channel = DMA_CHANNEL_0;
  hdma_spi3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_spi3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_spi3_rx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_spi3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_spi3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_spi3_rx.Init.Mode = DMA_CIRCULAR;
  hdma_spi3_rx.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_spi3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  if (HAL_DMA_Init(&hdma_spi3_rx) != HAL_OK)
  {
	  Error_Handler();
  }
  __HAL_LINKDMA(&hi2s3, hdmarx, hdma_spi3_rx);

  // Configure DMA for I2S3_TX
  hdma_spi2_tx.Instance = DMA1_Stream5;
  hdma_spi2_tx.Init.Channel = DMA_CHANNEL_0;
  hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_spi2_tx.Init.Mode = DMA_CIRCULAR;
  hdma_spi2_tx.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_spi2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  if (HAL_DMA_Init(&hdma_spi2_tx) != HAL_OK)
  {
	  Error_Handler();
  }
  __HAL_LINKDMA(&hi2s2, hdmatx, hdma_spi2_tx);


  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin
                          |LED8_Pin|SS_R_Pin|RESET_Pin|LED10_Pin
                          |LED11_Pin|LED12_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED9_Pin|SS_T_Pin|LED2_Pin|SW_E_Pin
                          |SW_S0_Pin|SW_S1_Pin|SW_S2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : SW2_S_Pin SW2_V__Pin SW2_V_C15_Pin SW4_S_Pin
                           SW4_V__Pin SW4_V_C12_Pin */
  GPIO_InitStruct.Pin = SW2_S_Pin|SW2_V__Pin|SW2_V_C15_Pin|SW4_S_Pin
                          |SW4_V__Pin|SW4_V_C12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED4_Pin LED5_Pin LED6_Pin LED7_Pin
                           LED8_Pin SS_R_Pin RESET_Pin LED10_Pin
                           LED11_Pin LED12_Pin */
  GPIO_InitStruct.Pin = LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin
                          |LED8_Pin|SS_R_Pin|RESET_Pin|LED10_Pin
                          |LED11_Pin|LED12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : SW3_S_Pin SW3_V__Pin SW3_V_A2_Pin */
  GPIO_InitStruct.Pin = SW3_S_Pin|SW3_V__Pin|SW3_V_A2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
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
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LED3_Pin */
  GPIO_InitStruct.Pin = LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED3_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
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
  while (1)
  {
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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

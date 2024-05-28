#include "main.h"

I2S_HandleTypeDef hi2s2;
I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_i2s2_rx;
DMA_HandleTypeDef hdma_i2s3_tx;

#define NUM_FRAMES 256
#define BUFFER_SIZE (NUM_FRAMES * sizeof(uint32_t))

uint32_t rxBuffer[NUM_FRAMES];
uint32_t txBuffer[NUM_FRAMES];
float volume = 0.5; // Volume factor (0.0 to 1.0)

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2S2_Init(void);
static void MX_I2S3_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_I2S2_Init();
    MX_I2S3_Init();

    // Start I2S2 in DMA mode (receive)
    if (HAL_I2S_Receive_DMA(&hi2s2, rxBuffer, NUM_FRAMES) != HAL_OK)
    {
        // Initialization Error
        Error_Handler();
    }

    // Start I2S3 in DMA mode (transmit)
    if (HAL_I2S_Transmit_DMA(&hi2s3, txBuffer, NUM_FRAMES) != HAL_OK)
    {
        // Initialization Error
        Error_Handler();
    }

    while (1)
    {
        // Main loop
    }
}

void MX_I2S2_Init(void)
{
    hi2s2.Instance = SPI2;
    hi2s2.Init.Mode = I2S_MODE_MASTER_RX;
    hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
    hi2s2.Init.DataFormat = I2S_DATAFORMAT_32B;
    hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
    hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_44K;
    hi2s2.Init.CPOL = I2S_CPOL_LOW;
    hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
    hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
    if (HAL_I2S_Init(&hi2s2) != HAL_OK)
    {
        Error_Handler();
    }
}

void MX_I2S3_Init(void)
{
    hi2s3.Instance = SPI3;
    hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
    hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
    hi2s3.Init.DataFormat = I2S_DATAFORMAT_32B;
    hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
    hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_44K;
    hi2s3.Init.CPOL = I2S_CPOL_LOW;
    hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
    hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
    if (HAL_I2S_Init(&hi2s3) != HAL_OK)
    {
        Error_Handler();
    }
}

void MX_DMA_Init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();

    // Configure DMA for I2S2_RX
    hdma_i2s2_rx.Instance = DMA1_Stream3;
    hdma_i2s2_rx.Init.Channel = DMA_CHANNEL_0;
    hdma_i2s2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_i2s2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_i2s2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_i2s2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_i2s2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_i2s2_rx.Init.Mode = DMA_CIRCULAR;
    hdma_i2s2_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_i2s2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_i2s2_rx) != HAL_OK)
    {
        Error_Handler();
    }
    __HAL_LINKDMA(&hi2s2, hdmarx, hdma_i2s2_rx);

    // Configure DMA for I2S3_TX
    hdma_i2s3_tx.Instance = DMA1_Stream5;
    hdma_i2s3_tx.Init.Channel = DMA_CHANNEL_0;
    hdma_i2s3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_i2s3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_i2s3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_i2s3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_i2s3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_i2s3_tx.Init.Mode = DMA_CIRCULAR;
    hdma_i2s3_tx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_i2s3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_i2s3_tx) != HAL_OK)
    {
        Error_Handler();
    }
    __HAL_LINKDMA(&hi2s3, hdmatx, hdma_i2s3_tx);
}

void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    // Add GPIO initialization code here
}

void SystemClock_Config(void)
{
    // Configure the system clock
}

void Error_Handler(void)
{
    // Handle errors
    while (1)
    {
    }
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI2)
    {
        // Manipulate the received data in rxBuffer before transmission
        for (int i = 0; i < NUM_FRAMES; i++)
        {
            int32_t sample = rxBuffer[i];
            sample = (sample >> 8) * volume; // Adjust volume and shift to 24-bit

            if (sample > 0x7FFFFF) sample = 0x7FFFFF;
            if (sample < -0x800000) sample = -0x800000;

            txBuffer[i] = sample << 8; // Shift back to 32-bit format
        }

        // Restart the I2S3 DMA transmit with the updated txBuffer
        if (HAL_I2S_Transmit_DMA(&hi2s3, txBuffer, NUM_FRAMES) != HAL_OK)
        {
            // Transmission Error
            Error_Handler();
        }
    }
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI3)
    {
        // Transmission complete, ready for next transfer if needed
        // For continuous transfer, you might reinitialize or handle buffer swapping here
    }
}

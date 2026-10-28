#include "main.h"
#include "encoder.h"
#include <stdio.h>
#include <string.h>

UART_HandleTypeDef huart1;

static void SystemClock_Config(void);
static void USART1_Init(void);

void Error_Handler(void)
{
    __disable_irq();
    while (1) { }
}

int main(void)
{
    EncData_t d;
    char buf[96];

    HAL_Init();
    SystemClock_Config();
    USART1_Init();
    Encoder_Init();

    const char *banner = "Encoder RPM - STM32F103 | 100 PPR, x4, 1 s\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t *)banner, strlen(banner), 100);

    while (1)
    {
        if (Encoder_GetData(&d))
        {
            const char *dir = (d.dir == ENC_DIR_CW)  ? "CW"  :
                              (d.dir == ENC_DIR_CCW) ? "CCW" : "STOP";

            int n = snprintf(buf, sizeof(buf),
                             "PULSE:%ld DIR:%s RPM:%lu.%02lu\r\n",
                             (long)d.total_pulses, dir,
                             (unsigned long)(d.rpm_x100 / 100U),
                             (unsigned long)(d.rpm_x100 % 100U));

            HAL_UART_Transmit(&huart1, (uint8_t *)buf, (uint16_t)n, 100);
        }
    }
}

static void USART1_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    gpio.Pin   = GPIO_PIN_9;                 /* TX */
    gpio.Mode  = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin  = GPIO_PIN_10;                 /* RX */
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio);

    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState       = RCC_HSE_ON;
    osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    osc.PLL.PLLState   = RCC_PLL_ON;
    osc.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLMUL     = RCC_PLL_MUL9;        /* 8 MHz x 9 = 72 MHz */
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) Error_Handler();

    clk.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;       /* 36 MHz, timer clk 72 MHz */
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

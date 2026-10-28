#include "encoder.h"

static TIM_HandleTypeDef htim3;
static TIM_HandleTypeDef htim4;

static volatile EncData_t enc_data;
static volatile uint8_t   enc_new;
static uint16_t           last_cnt;
static int32_t            total_counts;

extern void Error_Handler(void);

void Encoder_Init(void)
{
    GPIO_InitTypeDef gpio = {0};
    TIM_Encoder_InitTypeDef cfg = {0};

    __HAL_RCC_TIM3_CLK_ENABLE();
    htim3.Instance               = TIM3;
    htim3.Init.Prescaler         = 0;
    htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim3.Init.Period            = 0xFFFF;
    htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    cfg.EncoderMode  = TIM_ENCODERMODE_TI12;
    cfg.IC1Polarity  = TIM_ICPOLARITY_RISING;
    cfg.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    cfg.IC1Prescaler = TIM_ICPSC_DIV1;
    cfg.IC1Filter    = 6;
    cfg.IC2Polarity  = TIM_ICPOLARITY_RISING;
    cfg.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    cfg.IC2Prescaler = TIM_ICPSC_DIV1;
    cfg.IC2Filter    = 6;

    if (HAL_TIM_Encoder_Init(&htim3, &cfg) != HAL_OK) Error_Handler();


    __HAL_RCC_GPIOA_CLK_ENABLE();
    gpio.Pin  = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);

    if (HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL) != HAL_OK) Error_Handler();
    last_cnt = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);



    __HAL_RCC_TIM4_CLK_ENABLE();
    htim4.Instance               = TIM4;
    htim4.Init.Prescaler         = 7199;
    htim4.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim4.Init.Period            = 9999;
    htim4.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK) Error_Handler();

    HAL_NVIC_SetPriority(TIM4_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
    __HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE);
    if (HAL_TIM_Base_Start_IT(&htim4) != HAL_OK) Error_Handler();
}

void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim4);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM4) return;

    uint16_t now   = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);
    int16_t  delta = (int16_t)(now - last_cnt);
    last_cnt = now;
    total_counts += delta;

    uint32_t abs_delta = (delta < 0) ? (uint32_t)(-(int32_t)delta) : (uint32_t)delta;

    enc_data.delta_counts = delta;
    enc_data.total_pulses = total_counts / (int32_t)ENC_X4;

    if (delta == 0) {
        enc_data.rpm_x100 = 0;
        enc_data.dir      = ENC_DIR_STOP;
    } else {
        enc_data.rpm_x100 = (abs_delta * 6000U) / ENC_COUNTS_PER_REV;
        enc_data.dir      = (delta > 0) ? ENC_DIR_CW : ENC_DIR_CCW;
    }
    enc_new = 1;
}

uint8_t Encoder_GetData(EncData_t *out)
{
    if (!enc_new) return 0;
    __disable_irq();
    *out    = *(const EncData_t *)&enc_data;
    enc_new = 0;
    __enable_irq();
    return 1;
}

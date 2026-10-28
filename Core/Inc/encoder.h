#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define ENC_PPR              100U
#define ENC_X4               4U
#define ENC_COUNTS_PER_REV   (ENC_PPR * ENC_X4)

typedef enum {
    ENC_DIR_STOP = 0,
    ENC_DIR_CW,
    ENC_DIR_CCW
} EncDir_t;

typedef struct {
    int32_t  total_pulses;
    int32_t  delta_counts;
    uint32_t rpm_x100;
    EncDir_t dir;
} EncData_t;

void    Encoder_Init(void);
uint8_t Encoder_GetData(EncData_t *out);

#endif

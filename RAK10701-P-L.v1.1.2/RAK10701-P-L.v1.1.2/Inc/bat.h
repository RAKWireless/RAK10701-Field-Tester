#ifndef __BAT_H__
#define __BAT_H__

#include "common.h"

#define VBAT_MV_PER_LSB (0.73242188F) // 3.0V ADC range and 12 - bit ADC resolution = 3000mV / 4096
#define VBAT_DIVIDER_COMP (1.73)	  // Compensation factor for the VBAT divider, depend on the board
#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)
#define AIN_PIN WB_A0 // Potentiometer wiper (middle pin) connected to AIN0(J11 AIN)

void batInit();
uint8_t batRead(void);

#endif

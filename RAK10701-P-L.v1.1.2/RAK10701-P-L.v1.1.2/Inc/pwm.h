#ifndef __PWM_H__
#define __PWM_H__

#include <Arduino.h>

#define BL_PIN WB_IO3

#define PWM_FOSC 16000000 // The clock frequency of PWM is 16MHz.

void pwmInit();
void pwmBacklightFades(uint8_t duty, uint8_t backLight);
void setBackLight(uint8_t backLight);

#endif
#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "common.h"

#define BUTTON_INT_PIN WB_IO5

/*
 * @brief button state.
 */
typedef enum
{
	SINGLE_CLICK = 0,
	DOUBLE_CLICK,
	LONG_PRESS,
	BUTTONSTATE_NONE,
} buttonState_t;

initState_t buttonInit(void);
uint8_t getButtonStatus(void);

extern uint8_t g_screenOffFlag;

#endif
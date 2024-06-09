/**
   @file  main.h
   @author rakwireless.com
   @brief Used to include required header files.
   @version 0.1
   @date 2022-9-15
   @copyright Copyright (c) 2022
**/

#ifndef __MAIN_H__
#define __MAIN_H__

#include "./Inc/custom_at.h"
#include "./Inc/common.h"
#include "./Inc/lora.h"
#include "./Inc/bat.h"
#include "./Inc/gps.h"
#include "./Inc/tft.h"
#include "./Inc/Smooth_font.h"
#include "./Inc/tp.h"
#include "./Inc/pwm.h"
#include "./Inc/MillisTaskManager.h"
#include "./Inc/factoryTest.h"
#include "./Inc/button.h"

#include "nrf_sdh.h"
#include "atcmd.h"

// #include "nrf_gpio.h"

#define printTitle()                                             \
	{                                                            \
		Serial.println();                                        \
		Serial.println("====================================="); \
		Serial.printf("%s  %s\r\n", __DATE__, __TIME__);         \
		Serial.println("====================================="); \
	}

#endif

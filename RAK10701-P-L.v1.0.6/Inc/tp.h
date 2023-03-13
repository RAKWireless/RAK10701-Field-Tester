#ifndef __TP_H__
#define __TP_H__

#include "common.h"

#define RST_PIN   WB_IO5
#define INT_PIN   WB_IO6

#define XC1_POSITION1		(110)
#define XC1_POSITION2		(XC1_POSITION1 + 20)
#define XC2_POSITION1		(XC1_POSITION2)
#define XC2_POSITION2		(XC2_POSITION1 + 20)
#define XC3_POSITION1		(XC2_POSITION2)
#define XC3_POSITION2		(XC3_POSITION1 + 20)
#define XC4_POSITION1		(XC3_POSITION2)
#define XC4_POSITION2		(XC4_POSITION1 + 20)
#define XC5_POSITION1		(XC4_POSITION2)
#define XC5_POSITION2		(XC5_POSITION1 + 20)
#define XC6_POSITION1		(XC5_POSITION2)
#define XC6_POSITION2		(XC6_POSITION1 + 20)
#define XC7_POSITION1		(XC6_POSITION2)
#define XC7_POSITION2		(XC7_POSITION1 + 20)
#define XC8_POSITION1		(XC7_POSITION2)
#define XC8_POSITION2		(XC8_POSITION1 + 20)
#define XC9_POSITION1		(XC8_POSITION2)
#define XC9_POSITION2		(XC9_POSITION1 + 20)
#define XC10_POSITION1	(XC9_POSITION2)
#define XC10_POSITION2	(XC10_POSITION1 + 30)

#define DATE_Y1			  (70)
#define DATE_Y2			  (170)
#define DATE_W			  (23)


/*
 * @brief TP Area.
 */
typedef enum 
{
// Home Page.
  HOTSPOTS_AREA,
  RSSI_AREA,
	DISTANCE_AREA,
  SNR_AREA,
  GPS_AREA,
  SETTINGS_AREA,
	DISCOVERY_AREA,

// Sub Page.
  DATE1_AREA,
  DATE2_AREA,
  DATE3_AREA,
  DATE4_AREA,
  DATE5_AREA,
  DATE6_AREA,
  DATE7_AREA,
  DATE8_AREA,
  DATE9_AREA,
  DATE10_AREA,
  HOME_AREA,
	RETURN_AREA,
	
	BACK_AREA,
	RESTART_AREA,
	
	UP_SET_AREA,
	DOWN_SET_AREA,
	RIGHT_SET_AREA,
	OK_SET_AREA,
	
	BAND_SET_AREA,
	DR_SET_AREA,
	TXP_SET_AREA,
	TXI_SET_AREA,
	BL_SET_AREA,
	
	// POP_SHUTDOWN_AREA,
	// POP_RESTART_AREA,
	// POP_SCREEN_OFF_AREA,
	// POP_TOUCH_LOCK_AREA,
	// POP_EXIT_AREA,

  SHUTDOWN_AREA,
  // RESTART_AREA,
  
  ERROR_AREA,
}tpArea_t;

initState_t tpInit(void);
uint8_t getHomeTouchArea();
uint8_t getAppTouchArea();
uint8_t getTouchStatus();
void clearTouchStatus();
void tpMonitorMode();
uint8_t getTdStatus();

#endif

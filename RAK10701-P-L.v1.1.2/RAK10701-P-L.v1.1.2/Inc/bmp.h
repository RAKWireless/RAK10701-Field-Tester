#ifndef __BMP_H__
#define __BMP_H__

#include "common.h"

// Hotspots TP area.
#define HOTSPOTS_X1 (5)
#define HOTSPOTS_Y1 (50)
#define HOTSPOTS_X2 (HOTSPOTS_X1 + 100)
#define HOTSPOTS_Y2 (HOTSPOTS_Y1 + 127)

// Distance display area.
#define DISTANCE_X (114)
#define DISTANCE_Y (67)
#define TRIANGLE_X1 (DISTANCE_X + 56)
#define TRIANGLE_Y1 (DISTANCE_Y + 1)
// Distance TP area.
#define DISTANCE_X1 (110)
#define DISTANCE_Y1 (42)
#define DISTANCE_X2 (DISTANCE_X1 + 97)
#define DISTANCE_Y2 (DISTANCE_Y1 + 78)

// Rssi display area.
#define RSSI_X (212)
#define RSSI_Y (67)
#define TRIANGLE_X3 (RSSI_X + 30)
#define TRIANGLE_Y3 (RSSI_Y + 1)
// Rssi TP area.
#define RSSI_X1 (211)
#define RSSI_Y1 (42)
#define RSSI_X2 (RSSI_X1 + 97)
#define RSSI_Y2 (RSSI_Y1 + 78)

// Snr display area.
#define SNR_X (212)
#define SNR_Y (127)
#define TRIANGLE_X4 (SNR_X + 88)
#define TRIANGLE_Y4 (SNR_Y + 1)
// Snr TP area.
#define SNR_X1 (212)
#define SNR_Y1 (127)
#define SNR_X2 (SNR_X1 + 97)
#define SNR_Y2 (SNR_Y1 + 50)

// Gps display area.
#define LAT_X (16) // 60
#define LAT_Y (200)
#define LONG_X (LAT_X)
#define LONG_Y (LAT_Y + 16)
#define TRIANGLE_X5 (LAT_X + 120)
#define TRIANGLE_Y5 (LAT_Y + 1)
// Gps TP area.
#define GPS_X1 (55)
#define GPS_Y1 (190)
#define GPS_X2 (GPS_X1 + 115)
#define GPS_Y2 (GPS_Y1 + 50)

// Settings display area.
#define SET_X (278) // 16
#define SET_Y (197) // 197
// Settings TP area.
#define SETTINGS_X1 (270) // 0
#define SETTINGS_Y1 (188) // 190
#define SETTINGS_X2 (SETTINGS_X1 + 50)
#define SETTINGS_Y2 (SETTINGS_Y1 + 52)

// Settings display area.
#define DISCOVERY_X (233)
#define DISCOVERY_Y (197)
// Settings TP area.
#define DISCOVERY_X1 (220) // 0
#define DISCOVERY_Y1 (188) // 190
#define DISCOVERY_X2 (DISCOVERY_X1 + 49)
#define DISCOVERY_Y2 (DISCOVERY_Y1 + 52)

// Restart display area.
// #define RESTART_X     ( 77 )
// #define RESTART_Y		  ( 75 )
// // Restart TP area.
// #define RESTART_X1    ( 77  )
// #define RESTART_Y1    ( 75  )
// #define RESTART_X2    ( RESTART_X1 + 66 )
// #define RESTART_Y2    ( RESTART_Y1 + 66 )

// ShutDown display area.
#define SHUTDOWN_X (125) // 175
#define SHUTDOWN_Y (75)
// ShutDown TP area.
#define SHUTDOWN_X1 (125) // 175
#define SHUTDOWN_Y1 (75)
#define SHUTDOWN_X2 (SHUTDOWN_X1 + 71)
#define SHUTDOWN_Y2 (SHUTDOWN_Y1 + 71)

// Return display area.
#define RETURN_X (12)
#define RETURN_Y (200)
// Return TP area.
#define RETURN_X1 (0)
#define RETURN_Y1 (190)
#define RETURN_X2 (RETURN_X1 + 55)
#define RETURN_Y2 (RETURN_Y1 + 50)

// UP display area.
#define UP_X (79)
#define UP_Y (197)
// UP TP area.
#define UP_X1 (70)
#define UP_Y1 (190)
#define UP_X2 (UP_X1 + 50)
#define UP_Y2 (UP_Y1 + 50)

// DOWN display area.
#define DOWN_X (135)
#define DOWN_Y (197)
// DOWN TP area.
#define DOWN_X1 (125)
#define DOWN_Y1 (190)
#define DOWN_X2 (DOWN_X1 + 50)
#define DOWN_Y2 (DOWN_Y1 + 50)

// RIGHT display area.
#define RIGHT_X (190)
#define RIGHT_Y (197)
// RIGHT TP area.
#define RIGHT_X1 (180)
#define RIGHT_Y1 (190)
#define RIGHT_X2 (RIGHT_X1 + 50)
#define RIGHT_Y2 (RIGHT_Y1 + 50)

// OK display area.
#define OK_X (253)
#define OK_Y (198)
// OK TP area.
#define OK_X1 (240)
#define OK_Y1 (190)
#define OK_X2 (OK_X1 + 80)
#define OK_Y2 (OK_Y1 + 50)

// POP SHUTDOWN TP area.
#define POP_SHUTDOWN_X1 (276)
#define POP_SHUTDOWN_Y1 (189)
#define POP_SHUTDOWN_X2 (320)
#define POP_SHUTDOWN_Y2 (240)

// POP RESTART TP area.
#define POP_RESTART_X1 (225)
#define POP_RESTART_Y1 (189)
#define POP_RESTART_X2 (275)
#define POP_RESTART_Y2 (240)

// POP SCREEN OFF TP area.
#define POP_SCREEN_OFF_X1 (220)
#define POP_SCREEN_OFF_Y1 (180)
#define POP_SCREEN_OFF_X2 (262)
#define POP_SCREEN_OFF_Y2 (240)

// POP TOUCH LOCK TP area.
#define POP_TOUCH_LOCK_X1 (263)
#define POP_TOUCH_LOCK_Y1 (180)
#define POP_TOUCH_LOCK_X2 (320)
#define POP_TOUCH_LOCK_Y2 (240)

#define TFT_WIDTH (320)
#define TFT_HEIGHT (240)

#define SUBICO_W (26)
#define SUBICO_H (34)

#define BOOT_X1 (120)
#define BOOT_Y1 (40)

#define BOOT_X2 (117)
#define BOOT_Y2 (71)

#define BOOT_X3 (58)
#define BOOT_Y3 (170)

#define SEND_STATUS_X (160)
#define SEND_STATUS_Y (22)

#define GPS_X (225)
#define GPS_Y (10)

#define BLE_X (197)
#define BLE_Y (0)

#define BAT_X (265)
#define BAT_Y (10)

#define CHARGE_X (BAT_X + 16)
#define CHARGE_Y (BAT_Y + 2)

#define TRIANGLE_X2 (195)
#define TRIANGLE_Y2 (60)

#define APP_ICO_W (57)
#define APP_ICO_H (74)

#define HOME_ICO_W (57)
#define HOME_ICO_H (49)

#define HOME_X1 (131)
#define HOME_Y1 (191)
#define HOME_X2 (HOME_X1 + HOME_ICO_W)
#define HOME_Y2 (HOME_Y1 + HOME_ICO_H)

#define TXT_X (112)
#define TXT_Y1 (78)
#define TXT_Y2 (TXT_Y1 + 22)
#define TXT_Y3 (TXT_Y2 + 22)
#define TXT_Y4 (TXT_Y3 + 22)
#define TXT_Y5 (TXT_Y4 + 22)
#define TXT_Y6 (TXT_Y5 + 22)
#define TXT_Y7 (TXT_Y6 + 22)

#define DATE_X (61)
#define DATE_Y1 (70)
#define DATE_Y2 (170)
#define DATE_W (23)

#define RESTART_POP_X (72)
#define RESTART_POP_Y (80)

#ifndef GUI_CONST_STORAGE
#define GUI_CONST_STORAGE const
#endif

/**
 * @The data structure saved in Flash cannot exceed 128 bytes@NRF52840.
 */
typedef struct
{
	uint16_t xSize;
	uint16_t ySize;
	uint8_t bitsPerPixel;
	GUI_CONST_STORAGE unsigned short *date;
} GUI_BITMAP;

extern GUI_CONST_STORAGE GUI_BITMAP bmPopUp73X27;
extern GUI_CONST_STORAGE GUI_BITMAP bmPopLock18X23;
extern GUI_CONST_STORAGE GUI_BITMAP bmPopUnLock18X23;

extern GUI_CONST_STORAGE GUI_BITMAP bmUp31X31;
extern GUI_CONST_STORAGE GUI_BITMAP bmDown31X31;
extern GUI_CONST_STORAGE GUI_BITMAP bmRight31X31;
extern GUI_CONST_STORAGE GUI_BITMAP bmOk53X29;

extern GUI_CONST_STORAGE GUI_BITMAP bmReturn35X28;

extern GUI_CONST_STORAGE GUI_BITMAP bmSet32X30;
extern GUI_CONST_STORAGE GUI_BITMAP bmLock28X31;
extern GUI_CONST_STORAGE GUI_BITMAP bmDiscovery31X30;

// extern GUI_CONST_STORAGE GUI_BITMAP bmRestart66X66;
extern GUI_CONST_STORAGE GUI_BITMAP bmShutDown71X71;
extern GUI_CONST_STORAGE GUI_BITMAP bmNOHG86X81;

extern GUI_CONST_STORAGE GUI_BITMAP bmTriangle10X9;
extern GUI_CONST_STORAGE GUI_BITMAP bm1Triangle10X9;

extern GUI_CONST_STORAGE GUI_BITMAP bmBoot183X12;
extern GUI_CONST_STORAGE GUI_BITMAP bmBoot286X87;
extern GUI_CONST_STORAGE GUI_BITMAP bmBoot3204X45;

extern GUI_CONST_STORAGE GUI_BITMAP bmBat48X22;
extern GUI_CONST_STORAGE GUI_BITMAP bmCharge12X18;
extern GUI_CONST_STORAGE GUI_BITMAP bmGpsConnect25X25;
extern GUI_CONST_STORAGE GUI_BITMAP bmGpsInProgress38X25;
extern GUI_CONST_STORAGE GUI_BITMAP bmGpsNotConnect27X26;

extern GUI_CONST_STORAGE GUI_BITMAP bmRestartPop176X101;

#endif

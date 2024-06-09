#ifndef __TFT_H__
#define __TFT_H__

#include "common.h"
#include "../src/libraries/_Adafruit_ST7789.h" // Click here to get the library: http://librarymanager/All#Adafruit_ST7789
#include "../src/libraries/_Adafruit_GFX.h"			  // Click here to get the library: http://librarymanager/All#Adafruit_GFX
#include <SPI.h>

#define BL            WB_IO3
#define CS            WB_SPI_CS
#define RST           WB_IO5
#define DC            WB_IO4

#define TFT_BLACK     0x0000
#define TFT_WHITE     0xFFFF
#define TFT_DARKGREY  0x7BEF

#define DATE_COLOR		ST77XX_GREEN

#define MAX_LINE_COLOR	tft.color565(142,235,138)
#define MIN_LINE_COLOR	tft.color565(255,131,131)

#define MAX_DATE_COLOR	tft.color565(91,155,213)
#define MIN_DATE_COLOR	tft.color565(237,125,49)

#define TBG_COLOR				TFT_BLACK 						// top background color
#define MBG_COLOR				tft.color565(34,34,34)// middle background color
#define BBG_CLOR				TFT_BLACK

#define BAND_STR_X			20		
#define BAND_STR_Y			65	

#define DR_STR_X				(BAND_STR_X + 110)		
#define DR_STR_Y				BAND_STR_Y	

#define TXP_STR_X				BAND_STR_X		
#define TXP_STR_Y				(BAND_STR_Y	+ 16)

#define TXI_STR_X				(BAND_STR_X + 110)		
#define TXI_STR_Y				TXP_STR_Y

#define BL_STR_X				BAND_STR_X		
#define BL_STR_Y				(TXP_STR_Y	+ 16)

#define WM_STR_X				(BAND_STR_X + 110)		
#define WM_STR_Y				BL_STR_Y

#define VER_STR_X				BAND_STR_X		
#define VER_STR_Y				(BL_STR_Y	+ 16)

#define DEVEUI_STR_X		BAND_STR_X		
#define DEVEUI_STR_Y		(VER_STR_Y	+ 16)

#define APPEUI_STR_X		BAND_STR_X		
#define APPEUI_STR_Y		(DEVEUI_STR_Y	+ 16)

#define APPKEY_STR_X		BAND_STR_X		
#define APPKEY_STR_Y		(APPEUI_STR_Y	+ 16)

#define DEVEUI_X				DEVEUI_STR_X +58		
#define DEVEUI_Y				DEVEUI_STR_Y

#define APPEUI_X				APPEUI_STR_X +58		
#define APPEUI_Y				APPEUI_STR_Y

#define APPKEY_X				APPKEY_STR_X +58		
#define APPKEY_Y				APPKEY_STR_Y

#define BAND_X					(BAND_STR_X + 39)		
#define BAND_Y					BAND_STR_Y	

#define DR_X						(DR_STR_X + 25)			
#define DR_Y						DR_STR_Y	

#define TXP_X						(TXP_STR_X + 63)		
#define TXP_Y						TXP_STR_Y

#define TXI_X						(TXI_STR_X + 77)		
#define TXI_Y						TXI_STR_Y

#define BL_X						(BL_STR_X	+ 69)
#define BL_Y						BL_STR_Y	


/*
 * @brief Page num.
 */
typedef enum 
{
  BOOT_PAGE,
  HOME_PAGE,
  HOTSPOTS_PAGE,
  RSSI_PAGE,
	DISTANCE_PAGE,
  SNR_PAGE,
  GPS_PAGE,
  SETTINGS_PAGE,
	POPUP_PAGE,
	DISCOVERY_PAGE,
	SHUTDOWN_PAGE,
	RESTART_PAGE,
}page_t;


// extern uint8_t g_curr_band;
extern uint8_t g_discoveryMode;
// extern uint8_t g_paramChangeFlag;

void tftInit();
uint8_t getPageNum(void);
void bootPage();
/*
 * @brief Home Page.
 */
void refreshErroCode(int8_t date);
void refreshStatus(int8_t date);
void refreshSendStatus(int8_t date);
void refreshCharge(uint8_t date);
void refreshScreenLock(uint8_t date , uint8_t pageNum);

void refreshGps(uint8_t date);
void refreshBAT(uint8_t date);
void homePage();
void quitSubPage();
void refreshHomePage(gpsDate_t *gDate , hotspotsDate_t *hDate , distanceDate_t *dDate , rssiDate_t *rDate , snrDate_t *sDate);
void refreshGpsDate(gpsDate_t *gDate);
void subHomePage();
/*
 * @brief Sub Page.
 */
void hotSpotsPage(hotspotsDate_t *hdate);
void hotSpotsPageRefresh(hotspotsDate_t *hdate);
void hotSpotsPageDisplayLineValue(hotspotsDate_t *hdate,uint8_t pos);
 
// void displayHotSpotsLineValue(hotspotsDate_t *date,uint8_t pos);
// void hotSpotsPage(hotspotsDate_t *date);
// void refreshHotSpotsPage(hotspotsDate_t *date);

void rssiPage(hotspotsDate_t *hDate , rssiDate_t *rDate);
void rssiPageRefresh(hotspotsDate_t *hDate , rssiDate_t *rDate);
void rssiPageDisplayLineValue(hotspotsDate_t *hDate , rssiDate_t *date , uint8_t pos);

void distancePage(hotspotsDate_t *hDate , distanceDate_t *date);
void distancePageDisplayLineValue(hotspotsDate_t *hDate , distanceDate_t *date,uint8_t pos);
void distancePageRefresh(hotspotsDate_t *hDate , distanceDate_t *ddate);


void snrPage(hotspotsDate_t *hDate , snrDate_t *sDate);
void snrPageRefresh(hotspotsDate_t *hDate ,snrDate_t *sDate);
void snrPageDisplayLineValue(hotspotsDate_t *hDate ,snrDate_t *sDate,uint8_t pos);

// void gpsPage(gpsDate_t* date);
// void gpsPageUpdate(gpsDate_t* date);
void gpsPage(hotspotsDate_t *hDate ,gpsDate_t* gDate);
void gpsPageRefresh(hotspotsDate_t *hDate ,gpsDate_t* gDate);

void settingPage();
void settingPageRefresh();
void settingPageHandling(uint8_t button);
void setParam();
uint8_t getSetResult();
uint8_t getBandChangeStatus();

void popUpPage();
void refreshPopUpPage(uint8_t date );

void restartPage();

void discoveryPage();
void discoveryPageRefresh();
void quitDiscoveryPage();


void shutDownPage();
void quitShutDownPage();


void RAK14014_DeInit();

extern Adafruit_ST7789 tft;

extern uint8_t g_displayFlag;

extern uint8_t g_forceUplinkFlag;

#endif

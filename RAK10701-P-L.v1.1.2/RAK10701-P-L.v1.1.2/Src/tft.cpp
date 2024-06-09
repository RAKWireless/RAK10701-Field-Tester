
#include "../Inc/bmp.h"
#include "../Inc/tft.h"
#include "../Inc/Smooth_font.h"
#include "../Inc/custom_at.h"
#include "../Inc/NotoSansBold12.h"
#include "../Inc/NotoSansBold70.h"
#include "../Inc/bat.h"
#include "../Inc/lora.h"
#include "../Inc/gps.h"
#include "../Inc/pwm.h"
#include "../Inc/tp.h"
#include "../src/libraries/_qrcode.h"

Adafruit_ST7789 tft = Adafruit_ST7789(CS, DC, RST);

uint8_t g_displayFlag = 0;

static void bmpDraw(const unsigned char *arry, uint8_t x, uint8_t y);
static void drawBmp(const GUI_BITMAP *bmp , uint16_t x, uint16_t y);

static uint16_t read16FormArray(const unsigned char* p);
static uint32_t read32FormArray(const unsigned char* p);

static void hotSpotsPageUpdate(hotspotsDate_t *hDate);
static void rssiPageUpdate(hotspotsDate_t *hDate , rssiDate_t *rDate);
static void distancePageUpdate(hotspotsDate_t *hDate , distanceDate_t *ddate);
static void snrPageUpdate(hotspotsDate_t *hDate , snrDate_t *sDate);
static void gpsPageUpdate(hotspotsDate_t *hDate ,gpsDate_t* gDate);

static uint64_t arrayPosition = 0;

static uint8_t  page = BOOT_PAGE;
static uint8_t oldHotSpotsPos = 0; 
static uint8_t oldSnrPos = 0; 
static uint8_t oldRssiPos = 0; 
static uint8_t oldDistancePos = 0; 
static uint8_t oldBatVal = 254;

static uint8_t settingsArea = 0;
static uint8_t last_band = 0;
static uint8_t old_band  = 0;
uint8_t 			 curr_band = 0;

static uint8_t last_dr 	= 0;
static uint8_t curr_dr 	= 0;
static uint8_t curr_dr_max 	= 0;
static uint8_t curr_dr_min 	= 0;

static uint8_t last_txp 	= 0;
static uint8_t curr_txp 			= 0;
static uint8_t curr_txp_max 	= 0;
static uint8_t curr_txp_min 	= 0;

static uint8_t 	curr_bl 			= 0;
static uint32_t curr_txi			= 0;
static uint8_t flashCount 		= 0;

static uint8_t setBandResult 			= 1;
static uint8_t setDrResult 				= 1;
static uint8_t setTxpResult 			= 1;

uint8_t  g_forceUplinkFlag = 0;

void tftInit()
{
  tft.init(240, 320);
  tft.setRotation(3);
  
  tft.setTextSize(0);
	tft.fillScreen(TFT_BLACK);
	sf_loadFont(NotoSansBold12);
  sf_setCursor(0,0);
  sf_setTextWrap(true,true);
  sf_setTextColor(ST77XX_WHITE,ST77XX_BLACK,true);

	
	// digitalWrite(WB_IO3, HIGH);
	// while(1)
	// {
		// time_t timeout = millis();
		// sf_writeStr("J", strlen("J") ,LEFT_ALIGNED);
		// timeout = millis() - timeout;
		// Serial.printf("USE TIME: %dms\r\n",timeout);
		// delay(1000);
	// }
	
}

uint8_t getPageNum(void)
{
  return page;
}

void bootPage()
{
	page = BOOT_PAGE;
  tft.fillRect(0 , 0 , 320, 240, TFT_BLACK);
	
	drawBmp(&bmBoot183X12 ,BOOT_X1,BOOT_Y1);
	drawBmp(&bmBoot286X87 ,BOOT_X2,BOOT_Y2);
	drawBmp(&bmBoot3204X45,BOOT_X3,BOOT_Y3);
}

void refreshErroCode(int8_t date)
{
	sf_setCursor(170,12);
	sf_setTextColor(ST77XX_YELLOW,TFT_BLACK,false);
	switch(date)
	{
	case INIT_GPS_FAILED: 
		sf_writeStr("GPS INIT FAILED",20,RIGHT_ALIGNED);
		break;
	case INIT_TP_FAILED:
		sf_writeStr("TP INIT FAILED",20,RIGHT_ALIGNED);
		break;
	case INIT_FLASH_FAILED:
		sf_writeStr("FLASH INIT FAILED",20,RIGHT_ALIGNED);
		break;
	case INIT_LORA_FAILED:
		sf_writeStr("LORA INIT FAILED",20,RIGHT_ALIGNED);
		break;
	case INIT_BAT_REE:
		sf_writeStr("LOW VOLTAGE",20,RIGHT_ALIGNED);
		break;
	default:
		break;
	}
}

void refreshStatus(int8_t date)
{
	static uint8_t oldDate 		= 254;
  static uint8_t oldStatus 	= 254;
	static uint8_t status 		= 254;
	
	time_t passTime = millis() - g_sendTime;
	time_t displaySecond;
	time_t passSecond;
	static time_t oldPassSecond = 65536;
	
	char disp_str[20];
	memset(disp_str, 0, 20);
	
  if(oldDate != date)
  {
    oldDate = date;
		switch(date)
		{
			case LORA_JOINING:
			{
				sf_setCursor(69,16);
				tft.fillRoundRect(64, 10 , 60 , 23 , 2 ,tft.color565(0,106,198)); // ST77XX_BLUE
				sf_setTextColor(TFT_BLACK,tft.color565(0,106,198),false);
				sf_writeStr("JOINING",7,CENTER_ALIGNED);
				break;
			}
			case LORA_JOINED:
			{
				sf_setCursor(70,16);
				tft.fillRoundRect(64, 10 , 60 , 23,2 ,tft.color565(154,255,149)); // ST77XX_GREEN
				sf_setTextColor(TFT_BLACK,tft.color565(154,255,149),false);
				sf_writeStr("JOINED",7,CENTER_ALIGNED);
				break;
			}
			case LORA_JOIN_FAILED:
			{
				sf_setCursor(70,16);
				tft.fillRoundRect(64, 10 , 60 , 23,2 ,tft.color565(255,131,131));	// ST77XX_RED
				sf_setTextColor(TFT_BLACK,tft.color565(255,131,131),false);
				sf_writeStr("FAILED",7,CENTER_ALIGNED);
				break;
			}
			case LORA_SEND:
			{
				sf_setCursor(66,16);
				tft.fillRoundRect(64, 10 , 60 , 23,2 ,tft.color565(0,106,198)); // ST77XX_BLUE
				sf_setTextColor(TFT_BLACK,tft.color565(0,106,198),false);
				sf_writeStr("SENDING",7,CENTER_ALIGNED);
				break;
			}
			case LORA_IDLE:
			{
				sf_setCursor(70,16);
				tft.fillRoundRect(64, 10 , 60 , 23,2 ,tft.color565(181,181,181));	//	TFT_DARKGREY
				sf_setTextColor(TFT_BLACK,tft.color565(181,181,181),false);
				sf_writeStr("IDLE",7,CENTER_ALIGNED);
				break;
			}
			default:
				break;
		}
  }
	
	if(g_forceUplinkFlag == 1)
	{		
		g_forceUplinkFlag = 2;
		
		sprintf(disp_str, "                                  ");
		sf_setCursor(125,16);
		sf_setTextColor(TBG_COLOR,TBG_COLOR,false);
		sf_writeStr((const char*)disp_str,15,CENTER_ALIGNED);
		
		sprintf(disp_str, "Force Uplink");
		sf_setCursor(125,16);
		sf_setTextColor(tft.color565(154,255,149),TBG_COLOR,false);
		sf_writeStr((const char*)disp_str,15,CENTER_ALIGNED);
		oldStatus 	= 254;
		return;
	}
	else if(g_forceUplinkFlag == 2)
	{	
		return;
	}
	
	if(passTime<6000) // Send not completed, no data displayed.
	{
		oldStatus = status;
		status = 0;
	} 
	else
	{
		passSecond = (passTime/1000)/5;
		
		oldStatus = status;
		switch(getLoraSendSatus())
		{
			case SEND_NONE:
			{
				status = 0;
				break;
			}
			case SEND_FAILED:
			{
				status = 1;
				break;
			}
			case SEND_OK:
			{
				status = 2;
				break;
			}
			default:
				break;
		}
	}
  if(oldStatus == status && oldPassSecond == passSecond)
  {
    return;
  }
  else
  {
		oldPassSecond = passSecond;
  }
	displaySecond = passSecond * 5;
	switch(status)
	{
		case 0: // SEND_NONE
		{
			sprintf(disp_str, "                                  ");
			sf_setCursor(125,16);
			sf_setTextColor(TBG_COLOR,TBG_COLOR,false);
			sf_writeStr((const char*)disp_str,15,CENTER_ALIGNED);
			break;
		}
		case 1: // SEND_FAILED
		{
			if(displaySecond < 60)
			{
				sprintf(disp_str, "sent %ds ago", displaySecond);
			}
			else if(displaySecond < 3601)
			{
				sprintf(disp_str, "sent %dmin ago", (displaySecond/60));
			}
			else
			{
				sprintf(disp_str, "sent >60min ago");
			}
				
			sf_setCursor(125,16);
			sf_setTextColor(tft.color565(255,131,131),TBG_COLOR,false);
			sf_writeStr((const char*)disp_str,15,CENTER_ALIGNED);
			break;
		}
		case 2: // SEND_OK
		{
			if(displaySecond < 60)
			{
				sprintf(disp_str, "sent %ds ago", displaySecond);
			}
			else if(displaySecond < 3601)
			{
				sprintf(disp_str, "sent %dmin ago", (displaySecond/60));
			}
			else
			{
				sprintf(disp_str, "sent >60min ago");
			}
			
			sf_setCursor(125,16);
			sf_setTextColor(tft.color565(154,255,149),TBG_COLOR,false);
			sf_writeStr((const char*)disp_str,15,CENTER_ALIGNED);
			break;
		}
		default:
			break;
	}

}

void refreshSendStatus(int8_t date)
{
	// static uint8_t status = 254;
	// static uint8_t oldStatus = 254; // Can be any value you don't use.
	// time_t passTime = millis() - g_sendTime;
	// time_t displaySecond;
	// time_t passSecond;
	// static time_t oldPassSecond = 65536;
	// char disp_str[20];
	// memset(disp_str, 0, 20);
	
	// if(passTime<6000) // Send not completed, no data displayed.
	// {
		// oldStatus = status;
		// status = 0;
	// }
	// else
	// {
		// passSecond = (passTime/1000)/5;

		// switch(date)
		// {
			// case SEND_FAILED:
			// {
				// oldStatus = status;
				// status = 1;
				// break;
			// }
			// case SEND_OK:
			// {
				// oldStatus = status;
				// status = 2;
				// break;
			// }
			// case SEND_NONE:
			// {
				// oldStatus = status;
				// status = 0;
				// break;
			// }
			// default:
				// break;
		// }
	// }
  // if(oldStatus == status && oldPassSecond == passSecond)
  // {
    // return;
  // }
  // else
  // {
		// oldPassSecond = passSecond;
    // oldStatus = status;
  // }
	// displaySecond = passSecond * 5;
	// switch(status)
	// {
		// case 0: // SEND_NONE
		// {
			// sprintf(disp_str, "                              ");
			// sf_setCursor(128,16);
			// sf_setTextColor(TBG_COLOR,TBG_COLOR,false);
			// sf_writeStr((const char*)disp_str,15,CENTER_ALIGNED);
			// break;
		// }
		// case 1: // SEND_FAILED
		// {
			
			// if(displaySecond < 60)
			// {
				// sprintf(disp_str, "sent %ds ago", displaySecond);
			// }
			// else if(displaySecond < 3601)
			// {
				// sprintf(disp_str, "sent %dmin ago", (displaySecond/60));
			// }
			// else
			// {
				// sprintf(disp_str, "sent >60min ago");
			// }
				
			// sf_setCursor(128,16);
			// sf_setTextColor(tft.color565(255,131,131),TBG_COLOR,false);
			// sf_writeStr((const char*)disp_str,15,CENTER_ALIGNED);
			// break;
		// }
		// case 2: // SEND_OK
		// {
			// if(displaySecond < 60)
			// {
				// sprintf(disp_str, "sent %ds ago", displaySecond);
			// }
			// else if(displaySecond < 3601)
			// {
				// sprintf(disp_str, "sent %dmin ago", (displaySecond/60));
			// }
			// else
			// {
				// sprintf(disp_str, "sent >60min ago");
			// }
			
			// sf_setCursor(128,16);
			// sf_setTextColor(tft.color565(154,255,149),TBG_COLOR,false);
			// sf_writeStr((const char*)disp_str,15,CENTER_ALIGNED);
			// break;
		// }
		// default:
			// break;
	// }
}

static uint8_t changeIng = 0;
static uint8_t batValHistory[10];

void refreshCharge(uint8_t date)
{
  static uint8_t oldStete = 254;
  if(oldStete == date)
  {
    return;  // If the data is not updated, the picture will not be refreshed.
  }
  else
  {
    oldStete = date;
  }
  switch(date)
  {
    case 0:
    {
			changeIng = 0;
			uint8_t batVal = batRead();
			oldBatVal = 254;
			refreshBAT( batVal ); // Update battery power icon.
      break;
    }
    case 3:	// VBUS voltage above valid threshold and USBREG output settling time elapsed (same information as USBPWRRDY event)
    {
			changeIng = 1;
			memset(batValHistory , '0' , sizeof(batValHistory));
			tft.fillRect(BAT_X + 2 , BAT_Y + 2, 41, 18, TFT_BLACK);		// Fill power content is black.
			drawBmp(&bmCharge12X18,CHARGE_X,CHARGE_Y);
      break;
    }
    default:
      break;
  }
}

void refreshScreenLock(uint8_t date , uint8_t pageNum)
{
  static uint8_t oldStete = 254;
  if(oldStete == date)
  {
    return;  // If the data is not updated, the picture will not be refreshed.
  }
  else
  {
    oldStete = date;
  }
	
  switch(date)
  {
    case 0:
    {
			if(pageNum == HOME_PAGE)
			{
				tft.fillRect(SET_X , SET_Y, 32, 32, TFT_BLACK);
				drawBmp(&bmSet32X30,SET_X,SET_Y);
			}
			else
			{
				tft.fillRect(RETURN_X , RETURN_Y, 35, 35, TFT_BLACK);
				drawBmp(&bmReturn35X28,RETURN_X,RETURN_Y);
			}
      break;
    }
    case 1:
    {
			if(pageNum == HOME_PAGE)
			{
				tft.fillRect(SET_X , SET_Y, 32, 32, TFT_BLACK);
				drawBmp(&bmLock28X31,SET_X,SET_Y);
			}
			else
			{
				tft.fillRect(RETURN_X , RETURN_Y, 35, 35, TFT_BLACK);
				drawBmp(&bmLock28X31,RETURN_X,RETURN_Y);
			}
      break;
    }
    default:
      break;
  }
}

void refreshGps(uint8_t date)
{
  static uint8_t oldGpsVal = 254;
  if(oldGpsVal == date)
  {
    return;  // If the data is not updated, the picture will not be refreshed.
  }
  else
  {
    oldGpsVal = date;
  }
  switch(date)
  {
    case false:
    {
			drawBmp(&bmGpsInProgress38X25,GPS_X,GPS_Y);
      break;
    }
    case true:
    {
			tft.fillRect(GPS_X , GPS_Y, 38, 25, TFT_BLACK);
			drawBmp(&bmGpsConnect25X25,GPS_X,GPS_Y);
      break;
    }
    case 3:
    {
			tft.fillRect(GPS_X - 1 , GPS_Y, 39, 25, TFT_BLACK);
			drawBmp(&bmGpsNotConnect27X26,GPS_X,GPS_Y);
      break;
    }
    default:
      break;
  }
}

void refreshBAT(uint8_t date)
{	
	batValHistory[0] = date;
	
	for(int i = 10 - 1 ; i >0 ; i--)
	{
		batValHistory[i] = batValHistory[i-1];
	}
	
	if((batValHistory[0] == 4) && (batValHistory[1] == 4)\
	&& (batValHistory[2] == 4) && (batValHistory[3] == 4)\
	&& (batValHistory[4] == 4) && (batValHistory[5] == 4)\
	&& (batValHistory[6] == 4) && (batValHistory[7] == 4)\
	&& (batValHistory[8] == 4) && (batValHistory[9] == 4)\
	&& (changeIng == 1 )) // Fully charged and USB detected.  Filter out voltage jumps during charging.
	{
		// tft.fillRect(BAT_X + 2 , BAT_Y + 2, 40, 17, TFT_BLACK);		// Fill power content is black.
		tft.fillRect(BAT_X + 2 , BAT_Y + 2, 41, 18, ST77XX_GREEN);	// Fill the first grid with white.
		// tft.fillRect(BAT_X + 25 , BAT_Y + 15, 5 , 9, ST77XX_GREEN);	// Fill the second grid with white
		// tft.fillRect(BAT_X + 17 , BAT_Y + 15, 5 , 9, ST77XX_GREEN);	// Fill the third grid with white
		// tft.fillRect(BAT_X + 9  , BAT_Y + 15, 5 , 9, ST77XX_GREEN);	// Fill the fourth grid with white
		return;
	}
	
  if(oldBatVal == date)
  {
    return;  // If the data is not updated, the picture will not be refreshed.
  }
	else
  {
    oldBatVal = date;
  }
	
	if(changeIng != 1)
	{
		tft.fillRect(BAT_X + 2 , BAT_Y + 2, 41, 18, TFT_BLACK);		// Fill power content is black.
		switch(date)	// Charging not detected.
		{
			case 0:
			{
				break;
			}
			case 1:
			{
				tft.fillRect(BAT_X + 2 , BAT_Y + 2, 10 , 18, TFT_WHITE);	// Fill the first grid with white.
				break;
			}
			case 2:
			{
				tft.fillRect(BAT_X + 2 , BAT_Y + 2, 20 , 18, TFT_WHITE);	// Fill the first grid with white.
				break;
			}
			case 3:
			{
				tft.fillRect(BAT_X + 2 , BAT_Y + 2, 30 , 18, TFT_WHITE);	// Fill the first grid with white.
				break;
			}
			case 4:
			{
				tft.fillRect(BAT_X + 2 , BAT_Y + 2, 41 , 18, TFT_WHITE);	// Fill the first grid with white.
				break;
			}
			default:
				break;
		}
	}
}

void homePage()
{
	page = HOME_PAGE;
  tft.fillScreen(TFT_BLACK);
	
	sf_setCursor(16,16);
	sf_setTextColor(TFT_WHITE,TFT_BLACK,false);
  sf_writeStr("Status :",strlen("Status :"),LEFT_ALIGNED);
	tft.fillRect(0 , 40 , 320 , 147 , MBG_COLOR);
	
	drawBmp(&bmBat48X22,BAT_X,BAT_Y);
	
	// sf_loadFont(NotoSansBold70);
	// sf_setCursor(120,75);
	// sf_setTextColor(TFT_WHITE,TFT_BLACK);
  // sf_writeStr("15");
	
	subHomePage();
	
	// char disp_str[20];
	
	// sprintf(disp_str, "sent >60min ago");

	// sf_setCursor(125,16);
	// sf_setTextColor(tft.color565(154,255,149),TBG_COLOR,false);
	// sf_writeStr((const char*)disp_str,15,CENTER_ALIGNED);
	
	// delay(1000);

	// sprintf(disp_str, "                                  ");
	// sf_setCursor(125,16);
	// sf_setTextColor(TBG_COLOR,TBG_COLOR,false);
	// sf_writeStr((const char*)disp_str,15,CENTER_ALIGNED);
	
	// delay(1000);

	// memset(disp_str, 0, 20);
	// sprintf(disp_str, "Force Uplink");
	// sf_setCursor(125,16);
	// sf_setTextColor(tft.color565(154,255,149),TBG_COLOR,false);
	// sf_writeStr((const char*)disp_str,15,CENTER_ALIGNED);
	
	//delay(2000);
}

void refreshHomePage(gpsDate_t *gDate , hotspotsDate_t *hDate , distanceDate_t *dDate , rssiDate_t *rDate , snrDate_t *sDate)
{
	char disp_str[15];
	
	//if(rDate->count == 0 || ((gDate->hdop > 200) || (gDate->sats < 5)))
	//if((rDate->count == 0) || (g_displayFlag == 0))
	if(rDate->count == 0)
	{
		tft.fillRect(30, 85  , 56 , 44,MBG_COLOR);	//	Hotspots/Gateways
		memset(disp_str, 0, 15);
		sprintf(disp_str, " --");
		sf_loadFont(NotoSansBold70);
		sf_setCursor(30,82);
		sf_setTextColor(TFT_WHITE,MBG_COLOR,false);
		sf_writeStr((const char*)disp_str,2,CENTER_ALIGNED);

		sf_loadFont(NotoSansBold12);
		sf_setTextColor(TFT_WHITE,MBG_COLOR,true);
	
		// tft.fillRect(DISTANCE_X + 36 , DISTANCE_Y + 13 , 43 , 32,TFT_BLACK);	//	DISTANCE
		memset(disp_str, 0, 15);
		sprintf(disp_str, "--");
		sf_setCursor(DISTANCE_X + 38 , DISTANCE_Y + 17);
		sf_writeStr((const char*)disp_str,5,CENTER_ALIGNED);	
		sf_writeStr((const char*)"     ",1,LEFT_ALIGNED);
	
		memset(disp_str, 0, 15);
		sprintf(disp_str, "--");
		sf_setCursor(DISTANCE_X + 38 , DISTANCE_Y + 34);
		sf_writeStr((const char*)disp_str,5,CENTER_ALIGNED);
		sf_writeStr((const char*)" ",1,LEFT_ALIGNED);
	
		// tft.fillRect(RSSI_X + 36 , RSSI_Y + 13 , 30 , 32,TFT_BLACK);	//	RSSI	
		memset(disp_str, 0, 15);
		sprintf(disp_str, "--");
		sf_setCursor(RSSI_X + 38 , RSSI_Y + 17);
		sf_writeStr((const char*)disp_str,4,CENTER_ALIGNED);
		sf_writeStr((const char*)"   ",1,LEFT_ALIGNED);
	
		memset(disp_str, 0, 15);
		sprintf(disp_str, "--");
		sf_setCursor(RSSI_X + 38 , RSSI_Y + 34);
		sf_writeStr((const char*)disp_str,4,CENTER_ALIGNED);
		sf_writeStr((const char*)"   ",1,LEFT_ALIGNED);
	
		// tft.fillRect(SNR_X + 36 , SNR_Y - 4 , 30 , 16,TFT_BLACK);	//	SNR
		memset(disp_str, 0, 15);
		sprintf(disp_str, "--");
		sf_setCursor(SNR_X + 38 , SNR_Y);
		sf_writeStr((const char*)disp_str,4,CENTER_ALIGNED);
		sf_writeStr((const char*)"   ",1,LEFT_ALIGNED);
		return;
	}

	tft.fillRect(30, 85  , 56 , 44,MBG_COLOR);	//	Hotspots/Gateways

	memset(disp_str, 0, 15);
	sprintf(disp_str, "%d", hDate->origdate[0]);
	sf_loadFont(NotoSansBold70);
	sf_setCursor(30,82);
	sf_setTextColor(TFT_WHITE,MBG_COLOR,false);
	sf_writeStr((const char*)disp_str,2,CENTER_ALIGNED);
	
	sf_loadFont(NotoSansBold12);
	sf_setTextColor(TFT_WHITE,MBG_COLOR,true);
	
	// tft.fillRect(DISTANCE_X + 36 , DISTANCE_Y + 13 , 43 , 32,TFT_BLACK);	//	DISTANCE
	
	if((dDate->maxOrigdate[0] == 0) && (dDate->minOrigdate[0] == 0))
	{
		memset(disp_str, 0, 15);
		sprintf(disp_str, "--");
		sf_setCursor(DISTANCE_X + 38 , DISTANCE_Y + 17);
		sf_writeStr((const char*)disp_str,5,CENTER_ALIGNED);	
		sf_writeStr((const char*)"     ",1,LEFT_ALIGNED);
	
		memset(disp_str, 0, 15);
		sprintf(disp_str, "--");
		sf_setCursor(DISTANCE_X + 38 , DISTANCE_Y + 34);
		sf_writeStr((const char*)disp_str,5,CENTER_ALIGNED);
		sf_writeStr((const char*)" ",1,LEFT_ALIGNED);
	}
	else
	{
		memset(disp_str, 0, 15);
		sprintf(disp_str, "%d", dDate->maxOrigdate[0]);
		sf_setCursor(DISTANCE_X + 38 , DISTANCE_Y + 17);
		sf_writeStr((const char*)disp_str,5,RIGHT_ALIGNED);	
		sf_writeStr((const char*)" ",1,LEFT_ALIGNED);
		
		memset(disp_str, 0, 15);
		sprintf(disp_str, "%d", dDate->minOrigdate[0]);
		sf_setCursor(DISTANCE_X + 38 , DISTANCE_Y + 34);
		sf_writeStr((const char*)disp_str,5,RIGHT_ALIGNED);
		sf_writeStr((const char*)" ",1,LEFT_ALIGNED);
	}
	
	// tft.fillRect(RSSI_X + 36 , RSSI_Y + 13 , 30 , 32,TFT_BLACK);	//	RSSI	
	memset(disp_str, 0, 15);
	sprintf(disp_str, "%d", rDate->maxOrigdate[0]);
	sf_setCursor(RSSI_X + 38 , RSSI_Y + 17);
	sf_writeStr((const char*)disp_str,4,RIGHT_ALIGNED);
	sf_writeStr((const char*)" ",1,LEFT_ALIGNED);
	
	memset(disp_str, 0, 15);
	sprintf(disp_str, "%d", rDate->minOrigdate[0]);
	sf_setCursor(RSSI_X + 38 , RSSI_Y + 34);
	sf_writeStr((const char*)disp_str,4,RIGHT_ALIGNED);
	sf_writeStr((const char*)" ",1,LEFT_ALIGNED);
	
	// tft.fillRect(SNR_X + 36 , SNR_Y - 4 , 30 , 16,TFT_BLACK);	//	SNR
	memset(disp_str, 0, 15);
	sprintf(disp_str, "%d", sDate->origdate[0]);
	sf_setCursor(SNR_X + 38 , SNR_Y);
	sf_writeStr((const char*)disp_str,4,RIGHT_ALIGNED);
	sf_writeStr((const char*)" ",1,LEFT_ALIGNED);
}

void refreshGpsDate(gpsDate_t *gDate)
{
	char disp_str[15];
	if(page == HOME_PAGE)
	{
		// GPS positioning accuracy is good enough to display white, and yellow in positioning.
		// tft.fillRect(LAT_X + 37, LAT_Y , 70 , 26 , TFT_BLACK);	//	GPS	
		// if(g_displayFlag == 1)
		if((gDate->hdop <= 200) && (gDate->sats >=5)) 
		{
			sf_setTextColor(TFT_WHITE,TFT_BLACK , true);
			
			memset(disp_str, 0, 15);
			sprintf(disp_str, "%.6f", gDate->latitude / 10000000.0);
			sf_setCursor(LAT_X + 37,LAT_Y);
			sf_writeStr((const char*)disp_str,10,RIGHT_ALIGNED);
			sf_writeStr((const char*)" ",1,LEFT_ALIGNED);

			memset(disp_str, 0, 15);
			sprintf(disp_str, "%.6f", gDate->longitude / 10000000.0);
			sf_setCursor(LONG_X + 37,LONG_Y);
			sf_writeStr((const char*)disp_str,10,RIGHT_ALIGNED);
			sf_writeStr((const char*)" ",1,LEFT_ALIGNED);
		}
		else
		{
			sf_setTextColor(TFT_WHITE,TFT_BLACK,true);

			memset(disp_str, 0, 15);
			sprintf(disp_str, "  --");
			sf_setCursor(LAT_X + 37,LAT_Y);
			sf_writeStr((const char*)disp_str,10,CENTER_ALIGNED);
			sf_writeStr((const char*)"       ",1,LEFT_ALIGNED);

			memset(disp_str, 0, 15);
			sprintf(disp_str, "  --");
			sf_setCursor(LONG_X + 37,LONG_Y);
			sf_writeStr((const char*)disp_str,10,CENTER_ALIGNED);
			sf_writeStr((const char*)"       ",1,LEFT_ALIGNED);

			sf_setTextColor(TFT_WHITE,TFT_BLACK,false);
			refreshHomePage(gDate , &hotspotsDate, &distanceDate , &rssiDate , &snrDate);
		}
	}
	else if(page == GPS_PAGE)
	{
		
		
	}
	
}

void subHomePage()
{  
  page = HOME_PAGE;
	
	sf_setTextColor(TFT_WHITE,MBG_COLOR, false);
	sf_setCursor(16 ,	156); 
	// if(g_netWorkMode == 0)
	// {
		// sf_writeStr("No.of Gateway",strlen("No.of Gateway"),LEFT_ALIGNED);
	// }
	// else if(g_netWorkMode == 1)
	// {
		// sf_writeStr("No.of Hotspot",strlen("No.of Hotspot"),LEFT_ALIGNED);
	// }
	sf_writeStr("No.of Gateway",strlen("No.of Gateway"),LEFT_ALIGNED);
		
	// sf_setCursor(130,145);
  // sf_writeStr("Number of");
	// sf_setCursor(105,158);
	// sf_writeStr("Hotspots/Gateways");
	
	sf_setCursor(DISTANCE_X		,	DISTANCE_Y); 				sf_writeStr("Distance"	,strlen("Distance")	,LEFT_ALIGNED); 
	sf_setCursor(DISTANCE_X		,	DISTANCE_Y+17); 		sf_writeStr("MAX :"			,strlen("MAX :")		,LEFT_ALIGNED); 
	sf_setCursor(DISTANCE_X+78,	DISTANCE_Y+17); 		sf_writeStr("m"					,strlen("m")				,LEFT_ALIGNED);
	sf_setCursor(DISTANCE_X		,	DISTANCE_Y+34); 		sf_writeStr("MIN :"			,strlen("MIN :")		,LEFT_ALIGNED);
	sf_setCursor(DISTANCE_X+78,	DISTANCE_Y+34); 		sf_writeStr("m"					,strlen("m")				,LEFT_ALIGNED);
	
	sf_setCursor(RSSI_X				,	RSSI_Y); 						sf_writeStr("RSSI"			,strlen("RSSI")			,LEFT_ALIGNED);
	sf_setCursor(RSSI_X				,	RSSI_Y+17); 				sf_writeStr("MAX :"			,strlen("MAX :")		,LEFT_ALIGNED);
	sf_setCursor(RSSI_X+68		,	RSSI_Y+17); 				sf_writeStr("dBm"				,strlen("dBm")			,LEFT_ALIGNED);
	sf_setCursor(RSSI_X				,	RSSI_Y+34);  				sf_writeStr("MIN :"			,strlen("MIN :")		,LEFT_ALIGNED);
	sf_setCursor(RSSI_X+68		,	RSSI_Y+34);  				sf_writeStr("dBm"				,strlen("dBm")			,LEFT_ALIGNED);
	
	tft.drawLine(212, 118 , 307, 118 , TFT_WHITE);
	
	sf_setCursor(SNR_X				,	SNR_Y); 						sf_writeStr("SNR :"			,strlen("SNR :")		,LEFT_ALIGNED);
	sf_setCursor(SNR_X+68			,	SNR_Y); 						sf_writeStr("dB"				,strlen("dB")				,LEFT_ALIGNED);
	
	sf_setTextColor(TFT_WHITE , BBG_CLOR , false);
	sf_setCursor(LAT_X				,	LAT_Y); 						sf_writeStr("Lat"				,strlen("Lat")			,LEFT_ALIGNED);
	sf_setCursor(LAT_X+31			,	LAT_Y); 						sf_writeStr(":"					,strlen(":")				,LEFT_ALIGNED);
	sf_setCursor(LONG_X				,	LONG_Y); 						sf_writeStr("Long"			,strlen("Long")			,LEFT_ALIGNED);
	sf_setCursor(LONG_X+31		,	LONG_Y); 						sf_writeStr(":"					,strlen(":")				,LEFT_ALIGNED);
	
	drawBmp(&bmSet32X30,SET_X,SET_Y);
  //if(g_netWorkMode == 1)
  drawBmp(&bmDiscovery31X30,DISCOVERY_X,DISCOVERY_Y);
	
	//drawBmp(&bmShutDown30X30,SHUTDOWN_X,SHUTDOWN_Y);
	
	drawBmp(&bmNOHG86X81,15,65);

	drawBmp(&bm1Triangle10X9,TRIANGLE_X1,TRIANGLE_Y1); // Draw a triangle indicator.
	//drawBmp(&bmTriangle10X9,TRIANGLE_X2,TRIANGLE_Y2);
	drawBmp(&bm1Triangle10X9,TRIANGLE_X3,TRIANGLE_Y3);
	drawBmp(&bm1Triangle10X9,TRIANGLE_X4,TRIANGLE_Y4);
	drawBmp(&bmTriangle10X9,TRIANGLE_X5,TRIANGLE_Y5);
	
	tft.drawCircle(LAT_X + 110	, LAT_Y + 3	, 1, TFT_WHITE);
	tft.drawCircle(LONG_X + 110	, LONG_Y + 1, 1, TFT_WHITE);
	refreshHomePage(&gpsDate , &hotspotsDate, &distanceDate , &rssiDate , &snrDate);
	refreshGpsDate( &gpsDate );
}

void quitSubPage()
{
	tft.fillRect(0 , 40 , 320 , 147 , MBG_COLOR);
	tft.fillRect(0, 187 , 320 , 53  , TFT_BLACK);
}

void hotSpotsPage(hotspotsDate_t *hdate)
{
	page = HOTSPOTS_PAGE;
	
	char disp_str[15];
  oldHotSpotsPos = 0;

  drawBmp(&bmReturn35X28,RETURN_X,RETURN_Y);
  
  sf_setTextColor(TFT_WHITE,BBG_CLOR,false);
  sf_setCursor(107,212);
  // if(g_netWorkMode == 0)
  // {
    // sf_writeStr("No.of Gateway :",strlen("No.of Gateway :"),LEFT_ALIGNED);
  // }
  // else if(g_netWorkMode == 1)
  // {
    // sf_writeStr("No.of Hotspot :",strlen("No.of Hotspot :"),LEFT_ALIGNED);
  // }
	sf_writeStr("No.of Gateway :",strlen("No.of Gateway :"),LEFT_ALIGNED);
	
  // sf_writeStr("No.of Hotspot :",strlen("No.of Hotspot :"),LEFT_ALIGNED);
  
  sf_setTextColor(TFT_WHITE,MBG_COLOR,true);
  sf_setCursor(132,51);
	
  // sf_writeStr("HOTSPOT",strlen("HOTSPOT"),LEFT_ALIGNED);
	// if(g_netWorkMode == 0)
  // {
    // sf_writeStr("GATEWAY",strlen("GATEWAY"),LEFT_ALIGNED);
  // }
  // else if(g_netWorkMode == 1)
  // {
    // sf_writeStr("HOTSPOT",strlen("HOTSPOT"),LEFT_ALIGNED);
  // }
	sf_writeStr("GATEWAY",strlen("GATEWAY"),LEFT_ALIGNED);
	
  for(uint8_t i = 0; i < 4 ; i++) // Add ordinate values.[20,-30,-80,-130]
  {
    memset(disp_str, 0, 15);
    sf_setCursor(24,66 + i*26);
    sprintf(disp_str, "%d", 30 - i*(10));
    sf_writeStr((const char*)disp_str,2,RIGHT_ALIGNED);
  }
	
  for(uint8_t i = 0; i < 10 ; i++) // Add abscissa value.[10,9,8,7,6,5,4,3,2,1]
  {
    if(i == 0)
    {
      sf_setCursor(45, 166);  
    }
    else
    {
      sf_setCursor(52+i*27, 166); 
    }
    memset(disp_str, 0, 15);
    sprintf(disp_str, "%d", 10 - i);
    sf_writeStr((const char*)disp_str,1,RIGHT_ALIGNED);
  }
	hotSpotsPageUpdate(hdate);
}

static void hotSpotsPageUpdate(hotspotsDate_t *hDate)
{
	char disp_str[15];
	
	tft.fillRect(48, 70 , 253 , 81 ,MBG_COLOR);
	for(uint8_t i = 0; i < 4 ; i++) // Draw coordinate lines.
	{
		tft.drawLine(48, 70 + i*26 , 300 , 70 + i*26 , tft.color565(118,118,118));
	}
	for(uint8_t i = 0; i < hDate->count -1 ; i++) // Show line chart.
	{
		if(i!=8)
		{
			tft.drawLine(300-i*27, hDate->mapDate[i] , 300-(i+1)*27, hDate->mapDate[i+1] , TFT_WHITE);
		}
		else
		{
			tft.drawLine(300-i*27, hDate->mapDate[i] , 48, hDate->mapDate[i+1] , TFT_WHITE);
		}
	}
	if(hDate->count != 0)
	{
		if(oldHotSpotsPos != 9)
		{
			tft.drawLine(300-oldHotSpotsPos*27, 70 , 300-oldHotSpotsPos*27, 148 , tft.color565(0,106,198));
		}
		else
		{
			tft.drawLine(48, 70 , 48, 148 , tft.color565(0,106,198));
		}

		sf_setTextColor(TFT_WHITE,BBG_CLOR,true);

		sf_setCursor(203, 212);
		memset(disp_str, 0, 15);
		sprintf(disp_str, "%d", hDate->origdate[oldHotSpotsPos]);
		sf_writeStr((const char*)disp_str,3,LEFT_ALIGNED);
	}
}

void hotSpotsPageRefresh(hotspotsDate_t *hdate)
{
	if(hdate->updateState == true)
	{
		hdate->updateState = false;
		hotSpotsPageUpdate( hdate );
	}
}

void hotSpotsPageDisplayLineValue(hotspotsDate_t *hdate,uint8_t pos)
{
  if((oldHotSpotsPos == pos) || (pos >= hdate->count))
  {
    return;  
  }
	oldHotSpotsPos = pos;
	hotSpotsPageUpdate( hdate );
}

void rssiPage(hotspotsDate_t *hDate , rssiDate_t *rDate)
{
  page = RSSI_PAGE;
	char disp_str[15];
	oldRssiPos = 0;
	
	drawBmp(&bmReturn35X28,RETURN_X,RETURN_Y);
	
	sf_setTextColor(MAX_LINE_COLOR,BBG_CLOR,false);
	sf_setCursor(83,212);
	sf_writeStr("MAX :",strlen("MAX :"),LEFT_ALIGNED);
	
	sf_setTextColor(MIN_LINE_COLOR,BBG_CLOR,false);
	sf_setCursor(203,212);
	sf_writeStr("MIN :",strlen("MIN :"),LEFT_ALIGNED);
	
	sf_setTextColor(TFT_WHITE,MBG_COLOR,true);
	sf_setCursor(24,130);
	sf_writeStr("No.of",strlen("No.of"),LEFT_ALIGNED);
	sf_setCursor(17,143);
	// sf_writeStr("Hotspot",strlen("Hotspot"),LEFT_ALIGNED);
	// if(g_netWorkMode == 0)
  // {
    // sf_writeStr("Gateway",strlen("Gateway"),LEFT_ALIGNED);
  // }
  // else if(g_netWorkMode == 1)
  // {
    // sf_writeStr("Hotspot",strlen("Hotspot"),LEFT_ALIGNED);
  // }
	sf_writeStr("Gateway",strlen("Gateway"),LEFT_ALIGNED);
	sf_setCursor(190,50);
	sf_writeStr("RSSI",strlen("RSSI"),LEFT_ALIGNED);

	
  for(uint8_t i = 0; i < 4 ; i++) // Add ordinate values.[20,-30,-80,-130]
  {
		memset(disp_str, 0, 15);
		sf_setCursor(82,66 + i*26);
		sprintf(disp_str, "%d", i*(-50)+20);
    sf_writeStr((const char*)disp_str,4,RIGHT_ALIGNED);
  }
	
  for(uint8_t i = 0; i < 10 ; i++) // Add abscissa value.[10,9,8,7,6,5,4,3,2,1]
  {
    if(i == 0)
    {
      sf_setCursor(114, 166);  
    }
    else
    {
      sf_setCursor(117+i*20, 166); 
    }
		memset(disp_str, 0, 15);
		sprintf(disp_str, "%d", 10 - i);
    sf_writeStr((const char*)disp_str,1,RIGHT_ALIGNED);
  }
	rssiPageUpdate(hDate,rDate);
}

static void rssiPageUpdate(hotspotsDate_t *hDate , rssiDate_t *rDate)
{
	char disp_str[15];
	tft.fillRect(11, 76  , 56 , 44,MBG_COLOR);	//	Hotspots/Gateways
	memset(disp_str, 0, 15);
	sprintf(disp_str, "%d", hDate->origdate[oldRssiPos]);
	sf_loadFont(NotoSansBold70);
	sf_setCursor(12,76);
	sf_setTextColor(TFT_WHITE,MBG_COLOR,false);
	sf_writeStr((const char*)disp_str,2,CENTER_ALIGNED);
	sf_loadFont(NotoSansBold12);
	tft.fillRect(120, 70 , 182 , 78 ,MBG_COLOR);
	for(uint8_t i = 0; i < 4 ; i++) // Draw coordinate lines.
	{
		tft.drawLine(120, 70 + i*26 , 300, 70 + i*26 , tft.color565(118,118,118));
	}
	for(uint8_t i = 0; i < rDate->count -1 ; i++) // Show line chart.
	{
		if(i!=8)
		{
			tft.drawLine(300-i*20, rDate->maxMapDate[i] , 300-(i+1)*20, rDate->maxMapDate[i+1] , MAX_LINE_COLOR);
			tft.drawLine(300-i*20, rDate->minMapDate[i] , 300-(i+1)*20, rDate->minMapDate[i+1] , MIN_LINE_COLOR);
		}
		else
		{
			tft.drawLine(300-i*20, rDate->maxMapDate[i] , 120, rDate->maxMapDate[i+1] , MAX_LINE_COLOR);
			tft.drawLine(300-i*20, rDate->minMapDate[i] , 120, rDate->minMapDate[i+1] , MIN_LINE_COLOR);
		}
	}
	if(rDate->count != 0)
	{
		if(oldRssiPos != 9)
		{
			tft.drawLine(300-oldRssiPos*20, 70 , 300-oldRssiPos*20, 148 , tft.color565(0,106,198));
		}
		else
		{
			tft.drawLine(120, 70 , 120, 148 , tft.color565(0,106,198));
		}

		sf_setTextColor(TFT_WHITE,BBG_CLOR,true);

		sf_setCursor(120, 212);
		memset(disp_str, 0, 15);
		sprintf(disp_str, "%ddBm", rDate->maxOrigdate[oldRssiPos]);
		sf_writeStr((const char*)disp_str,8,LEFT_ALIGNED);

		sf_setCursor(240, 212);
		memset(disp_str, 0, 15);
		sprintf(disp_str, "%ddBm", rDate->minOrigdate[oldRssiPos]);
		sf_writeStr((const char*)disp_str,8,LEFT_ALIGNED);
	}
}

void rssiPageRefresh(hotspotsDate_t *hDate , rssiDate_t *rDate)
{
	if(rDate->updateState == true)
	{	
		rDate->updateState = false;
		rssiPageUpdate(hDate,rDate);
	}
}

void rssiPageDisplayLineValue(hotspotsDate_t *hDate , rssiDate_t *rDate , uint8_t pos)
{
  if((oldRssiPos == pos) || (pos >= rDate->count))
  {
    return;  
  }
	oldRssiPos = pos;
	rssiPageUpdate(hDate , rDate);
	
	
  // if((oldRssiPos == pos) || (pos >= date->count))
  // {
    // return;  
  // }
  // tft.fillCircle(DATE_X+oldRssiPos*DATE_W, date->maxMapDate[oldRssiPos], 3, TFT_BLACK);
	// tft.fillCircle(DATE_X+oldRssiPos*DATE_W, date->minMapDate[oldRssiPos], 3, TFT_BLACK);
  // for(uint8_t i = 0; i < 4 ; i++) // Draw coordinate lines.
  // {
    // tft.drawLine(45, 70 + i*30 , 300, 70 + i*30 , tft.color565(84,84,84));
  // }
  // for(uint8_t i = 0; i < date->count -1 ; i++) // Show line chart.
  // {
    // tft.drawLine(61+i*23, date->maxMapDate[i] , 61+(i+1)*23, date->maxMapDate[i+1] , MAX_LINE_COLOR);
		// tft.drawLine(61+i*23, date->minMapDate[i] , 61+(i+1)*23, date->minMapDate[i+1] , MIN_LINE_COLOR);
  // }
  // tft.fillCircle(DATE_X+pos*DATE_W, date->maxMapDate[pos], 3, tft.color565(34,58,80));
  // tft.fillCircle(DATE_X+pos*DATE_W, date->maxMapDate[pos], 2, tft.color565(91,133,171));
  // tft.fillCircle(DATE_X+pos*DATE_W, date->minMapDate[pos], 3, tft.color565(34,58,80));
  // tft.fillCircle(DATE_X+pos*DATE_W, date->minMapDate[pos], 2, tft.color565(91,133,171));
  // tft.fillRect(50, 45 , 255 , 20,TFT_BLACK);
  // tft.setCursor(50, 60);
	// tft.setTextColor(MAX_DATE_COLOR, TFT_BLACK);
	// tft.print("max: ");
  // tft.print(date->maxOrigdate[pos]);
	// tft.print(" (dBm)");

	// tft.setTextColor(MIN_DATE_COLOR, TFT_BLACK);
	// tft.printf(" min: ");
	// tft.print(date->minOrigdate[pos]);
	// tft.print(" (dBm)");
	// oldRssiPos = pos;
}



void distancePage(hotspotsDate_t *hDate ,distanceDate_t *ddate)
{
  page = DISTANCE_PAGE;
	char disp_str[15];
  oldDistancePos = 0;
	
	drawBmp(&bmReturn35X28,RETURN_X,RETURN_Y);
	
	sf_setTextColor(TFT_WHITE,BBG_CLOR,false);
	sf_setCursor(83,212);
	sf_writeStr("MAX :",strlen("MAX :"),LEFT_ALIGNED);
	
	sf_setCursor(203,212);
	sf_writeStr("MIN :",strlen("MIN :"),LEFT_ALIGNED);
	
	sf_setTextColor(TFT_WHITE,MBG_COLOR,true);
	sf_setCursor(24,130);
	sf_writeStr("No.of",strlen("No.of"),LEFT_ALIGNED);
	sf_setCursor(17,143);
	// sf_writeStr("Hotspot",strlen("Hotspot"),LEFT_ALIGNED);
	// if(g_netWorkMode == 0)
  // {
    // sf_writeStr("Gateway",strlen("Gateway"),LEFT_ALIGNED);
  // }
  // else if(g_netWorkMode == 1)
  // {
    // sf_writeStr("Hotspot",strlen("Hotspot"),LEFT_ALIGNED);
  // }
	sf_writeStr("Gateway",strlen("Gateway"),LEFT_ALIGNED);
	sf_setCursor(185,50);
	sf_writeStr("DISTANCE",strlen("DISTANCE"),LEFT_ALIGNED);
	
  for(uint8_t i = 0; i < 4 ; i++) // Add ordinate values.[20,-30,-80,-130]
  {
		memset(disp_str, 0, 15);
		sf_setCursor(82,66 + i*26);
		sprintf(disp_str, "%d", i*(-11)+33);
    sf_writeStr((const char*)disp_str,4,RIGHT_ALIGNED);
  }
	
  for(uint8_t i = 0; i < 10 ; i++) // Add abscissa value.[10,9,8,7,6,5,4,3,2,1]
  {
    if(i == 0)
    {
      sf_setCursor(114, 166);  
    }
    else
    {
      sf_setCursor(117+i*20, 166); 
    }
		memset(disp_str, 0, 15);
		sprintf(disp_str, "%d", 10 - i);
    sf_writeStr((const char*)disp_str,1,RIGHT_ALIGNED);
  }
	distancePageUpdate(hDate,ddate);
}

static void distancePageUpdate(hotspotsDate_t *hDate , distanceDate_t *ddate)
{
	char disp_str[15];
	tft.fillRect(11, 76  , 56 , 44,MBG_COLOR);	//	Hotspots/Gateways
	memset(disp_str, 0, 15);
	sprintf(disp_str, "%d", hDate->origdate[oldDistancePos]);
	sf_loadFont(NotoSansBold70);
	sf_setCursor(12,76);
	sf_setTextColor(TFT_WHITE,MBG_COLOR,false);
	sf_writeStr((const char*)disp_str,2,CENTER_ALIGNED);
	sf_loadFont(NotoSansBold12);
	tft.fillRect(120, 70 , 182 , 78 ,MBG_COLOR);
	for(uint8_t i = 0; i < 4 ; i++) // Draw coordinate lines.
	{
		tft.drawLine(120, 70 + i*26 , 300, 70 + i*26 , tft.color565(118,118,118));
	}
	for(uint8_t i = 0; i < ddate->count -1 ; i++) // Show line chart.
	{
		if(i!=8)
		{
			tft.drawLine(300-i*20, ddate->maxMapDate[i] , 300-(i+1)*20, ddate->maxMapDate[i+1] , TFT_WHITE);
			tft.drawLine(300-i*20, ddate->minMapDate[i] , 300-(i+1)*20, ddate->minMapDate[i+1] , TFT_WHITE);
		}
		else
		{
			tft.drawLine(300-i*20, ddate->maxMapDate[i] , 120, ddate->maxMapDate[i+1] , TFT_WHITE);
			tft.drawLine(300-i*20, ddate->minMapDate[i] , 120, ddate->minMapDate[i+1] , TFT_WHITE);
		}
	}
	if(ddate->count != 0)
	{
		if(oldDistancePos != 9)
		{
			tft.drawLine(300-oldDistancePos*20, 70 , 300-oldDistancePos*20, 148 , tft.color565(0,106,198));
		}
		else
		{
			tft.drawLine(120, 70 , 120, 148 , tft.color565(0,106,198));
		}

		sf_setTextColor(TFT_WHITE,BBG_CLOR,true);

		sf_setCursor(120, 212);
		memset(disp_str, 0, 15);
		sprintf(disp_str, "%dm", ddate->maxOrigdate[oldDistancePos]);
		sf_writeStr((const char*)disp_str,8,LEFT_ALIGNED);

		sf_setCursor(240, 212);
		memset(disp_str, 0, 15);
		sprintf(disp_str, "%dm", ddate->minOrigdate[oldDistancePos]);
		sf_writeStr((const char*)disp_str,8,LEFT_ALIGNED);
	}
}
void distancePageRefresh(hotspotsDate_t *hDate , distanceDate_t *ddate)
{
	if(ddate->updateState == true)
	{
		ddate->updateState = false;
		distancePageUpdate(hDate,ddate);
	}
	
	// if(date->updateState == true)
	// {
		// date->updateState = false;
		// tft.fillRect(45, 67 , 255 , 96,TFT_BLACK);  // Overlay table display area with black.
		// for(uint8_t i = 0; i < 4 ; i++) // Draw coordinate lines.
		// {
			// tft.drawLine(45, 70 + i*30 , 300, 70 + i*30 , tft.color565(84,84,84));
		// }
		// for(uint8_t i = 0; i < date->count -1 ; i++) // Show line chart.
		// {
			// tft.drawLine(61+i*23, date->maxMapDate[i] , 61+(i+1)*23, date->maxMapDate[i+1] , MAX_LINE_COLOR);
			// tft.drawLine(61+i*23, date->minMapDate[i] , 61+(i+1)*23, date->minMapDate[i+1] , MIN_LINE_COLOR);
		// }
		// tft.fillCircle(DATE_X+oldDistancePos*DATE_W, date->maxMapDate[oldDistancePos], 3, tft.color565(34,58,80));
		// tft.fillCircle(DATE_X+oldDistancePos*DATE_W, date->maxMapDate[oldDistancePos], 2, tft.color565(91,133,171));
		// tft.fillCircle(DATE_X+oldDistancePos*DATE_W, date->minMapDate[oldDistancePos], 3, tft.color565(34,58,80));
		// tft.fillCircle(DATE_X+oldDistancePos*DATE_W, date->minMapDate[oldDistancePos], 2, tft.color565(91,133,171));
		// tft.fillRect(50, 45 , 255 , 20,TFT_BLACK);
		// tft.setCursor(50, 60);
		// tft.setTextColor(MAX_DATE_COLOR, TFT_BLACK);
		// tft.print("max: ");
		// tft.print(date->maxOrigdate[oldDistancePos]);
		// tft.print(" (m)");

		// tft.setTextColor(MIN_DATE_COLOR, TFT_BLACK);
		// tft.print(" min: ");
		// tft.print(date->minOrigdate[oldDistancePos]);
		// tft.print(" (m)");
	// }
}

void distancePageDisplayLineValue(hotspotsDate_t *hDate , distanceDate_t *ddate,uint8_t pos)
{
	if((oldDistancePos == pos) || (pos >= ddate->count))
  {
    return;  
  }
	oldDistancePos = pos;
	distancePageUpdate(hDate , ddate);
	
  // if((oldDistancePos == pos) || (pos >= date->count))
  // {
    // return;  
  // }
  // tft.fillCircle(DATE_X+oldDistancePos*DATE_W, date->maxMapDate[oldDistancePos], 3, TFT_BLACK);
	// tft.fillCircle(DATE_X+oldDistancePos*DATE_W, date->minMapDate[oldDistancePos], 3, TFT_BLACK);
  // for(uint8_t i = 0; i < 4 ; i++) // Draw coordinate lines.
  // {
    // tft.drawLine(45, 70 + i*30 , 300, 70 + i*30 , tft.color565(84,84,84));
  // }
  // for(uint8_t i = 0; i < date->count -1 ; i++) // Show line chart.
  // {
    // tft.drawLine(61+i*23, date->maxMapDate[i] , 61+(i+1)*23, date->maxMapDate[i+1] , MAX_LINE_COLOR);
		// tft.drawLine(61+i*23, date->minMapDate[i] , 61+(i+1)*23, date->minMapDate[i+1] , MIN_LINE_COLOR);
  // }
  // tft.fillCircle(DATE_X+pos*DATE_W, date->maxMapDate[pos], 3, tft.color565(34,58,80));
  // tft.fillCircle(DATE_X+pos*DATE_W, date->maxMapDate[pos], 2, tft.color565(91,133,171));
  // tft.fillCircle(DATE_X+pos*DATE_W, date->minMapDate[pos], 3, tft.color565(34,58,80));
  // tft.fillCircle(DATE_X+pos*DATE_W, date->minMapDate[pos], 2, tft.color565(91,133,171));
  // tft.fillRect(50, 45 , 255 , 20,TFT_BLACK);
  // tft.setCursor(50, 60);
	// tft.setTextColor(MAX_DATE_COLOR, TFT_BLACK);
	// tft.print("max: ");
  // tft.print(date->maxOrigdate[pos]);
	// tft.print(" (m)");

	// tft.setTextColor(MIN_DATE_COLOR, TFT_BLACK);
	// tft.print(" min: ");
	// tft.print(date->minOrigdate[pos]);
	// tft.print(" (m)");
	// oldDistancePos = pos;
}


void snrPage(hotspotsDate_t *hDate , snrDate_t *sDate)
{
	page = SNR_PAGE;
	
	char disp_str[15];
	oldSnrPos = 0;
	
	drawBmp(&bmReturn35X28,RETURN_X,RETURN_Y);
	
	sf_setTextColor(TFT_WHITE,BBG_CLOR,false);
	sf_setCursor(122,212);
	sf_writeStr("SNR :",strlen("SNR :"),LEFT_ALIGNED);
		
	sf_setTextColor(TFT_WHITE,MBG_COLOR,true);
	sf_setCursor(24,130);
	sf_writeStr("No.of",strlen("No.of"),LEFT_ALIGNED);
	sf_setCursor(17,143);
	// sf_writeStr("Hotspot",strlen("Hotspot"),LEFT_ALIGNED);
	// if(g_netWorkMode == 0)
  // {
    // sf_writeStr("Gateway",strlen("Gateway"),LEFT_ALIGNED);
  // }
  // else if(g_netWorkMode == 1)
  // {
    // sf_writeStr("Hotspot",strlen("Hotspot"),LEFT_ALIGNED);
  // }
	sf_writeStr("Gateway",strlen("Gateway"),LEFT_ALIGNED);
	sf_setCursor(190,50);
	sf_writeStr("SNR",strlen("SNR"),LEFT_ALIGNED);

	
  for(uint8_t i = 0; i < 4 ; i++) // Add ordinate values.[20,-30,-80,-130]
  {
		memset(disp_str, 0, 15);
		sf_setCursor(82,66 + i*26);
		sprintf(disp_str, "%d", i*(-20)+30);
    sf_writeStr((const char*)disp_str,4,RIGHT_ALIGNED);
  }
	
  for(uint8_t i = 0; i < 10 ; i++) // Add abscissa value.[10,9,8,7,6,5,4,3,2,1]
  {
    if(i == 0)
    {
      sf_setCursor(114, 166);  
    }
    else
    {
      sf_setCursor(117+i*20, 166); 
    }
		memset(disp_str, 0, 15);
		sprintf(disp_str, "%d", 10 - i);
    sf_writeStr((const char*)disp_str,1,RIGHT_ALIGNED);
  }
	snrPageUpdate(hDate,sDate);
}

static void snrPageUpdate(hotspotsDate_t *hDate , snrDate_t *sDate)
{
	char disp_str[15];
	tft.fillRect(11, 76  , 56 , 44,MBG_COLOR);	//	Hotspots/Gateways
	memset(disp_str, 0, 15);
	sprintf(disp_str, "%d", hDate->origdate[oldSnrPos]);
	sf_loadFont(NotoSansBold70);
	sf_setCursor(12,76);
	sf_setTextColor(TFT_WHITE,MBG_COLOR,false);
	sf_writeStr((const char*)disp_str,2,CENTER_ALIGNED);
	sf_loadFont(NotoSansBold12);
	tft.fillRect(120, 70 , 182 , 78 ,MBG_COLOR);
	for(uint8_t i = 0; i < 4 ; i++) // Draw coordinate lines.
	{
		tft.drawLine(120, 70 + i*26 , 300, 70 + i*26 , tft.color565(118,118,118));
	}
	for(uint8_t i = 0; i < sDate->count -1 ; i++) // Show line chart.
	{
		if(i!=8)
		{
			tft.drawLine(300-i*20, sDate->mapDate[i] , 300-(i+1)*20, sDate->mapDate[i+1] , TFT_WHITE);
		}
		else
		{
			tft.drawLine(300-i*20, sDate->mapDate[i] , 120, sDate->mapDate[i+1] , TFT_WHITE);
		}
	}
	if(sDate->count != 0)
	{
		if(oldSnrPos != 9)
		{
			tft.drawLine(300-oldSnrPos*20, 70 , 300-oldSnrPos*20, 148 , tft.color565(0,106,198));
		}
		else
		{
			tft.drawLine(120, 70 , 120, 148 , tft.color565(0,106,198));
		}

		sf_setTextColor(TFT_WHITE,BBG_CLOR,true);

		sf_setCursor(159, 212);
		memset(disp_str, 0, 15);
		sprintf(disp_str, "%ddB", sDate->origdate[oldSnrPos]);
		sf_writeStr((const char*)disp_str,8,LEFT_ALIGNED);
	}
}

void snrPageRefresh(hotspotsDate_t *hDate ,snrDate_t *sDate)
{
	if(sDate->updateState == true)
	{	
		sDate->updateState = false;
		snrPageUpdate(hDate,sDate);
	}
}

void snrPageDisplayLineValue(hotspotsDate_t *hDate ,snrDate_t *sDate,uint8_t pos)
{
	if((oldSnrPos == pos) || (pos >= sDate->count))
  {
    return;  
  }
	oldSnrPos = pos;
	snrPageUpdate(hDate , sDate);
}

void gpsPage(hotspotsDate_t *hDate ,gpsDate_t* gDate)
{
  page = GPS_PAGE;

	drawBmp(&bmReturn35X28,RETURN_X,RETURN_Y);
		
	sf_setTextColor(TFT_WHITE,MBG_COLOR,true);
	sf_setCursor(24,130);
	sf_writeStr("No.of",strlen("No.of"),LEFT_ALIGNED);
	sf_setCursor(17,143);
	// sf_writeStr("Hotspot",strlen("Hotspot"),LEFT_ALIGNED);
	
  // if(g_netWorkMode == 0)
  // {
    // sf_writeStr("Gateway",strlen("Gateway"),LEFT_ALIGNED);
  // }
  // else if(g_netWorkMode == 1)
  // {
    // sf_writeStr("Hotspot",strlen("Hotspot"),LEFT_ALIGNED);
  // }
	
	sf_writeStr("Gateway",strlen("Gateway"),LEFT_ALIGNED);
	
	sf_setCursor(175,50);
	sf_writeStr("GPS",strlen("GPS"),LEFT_ALIGNED);
	
	sf_setCursor(TXT_X+6, TXT_Y1);
	sf_writeStr("Latitude :", 11 ,RIGHT_ALIGNED);
	
	sf_setCursor(TXT_X, TXT_Y2);
	sf_writeStr("Longitude :", 11 ,RIGHT_ALIGNED);
	
	sf_setCursor(TXT_X+7, TXT_Y3);
	sf_writeStr("Altitude :", 11 ,RIGHT_ALIGNED);
	
	sf_setCursor(TXT_X, TXT_Y4);
	sf_writeStr("Hdop :", 11 ,RIGHT_ALIGNED);
	
	sf_setCursor(TXT_X+8, TXT_Y5);
	sf_writeStr("Sats :", 11 ,RIGHT_ALIGNED);
	
	tft.drawCircle(272	, 102	, 1, TFT_WHITE);
	tft.drawCircle(272  , 80  , 1, TFT_WHITE);
	
	gpsPageUpdate(hDate , gDate);
}

static void gpsPageUpdate(hotspotsDate_t *hDate ,gpsDate_t* gDate)
{
	char disp_str[50] = {0};
	
	tft.fillRect(11, 76  , 56 , 44,MBG_COLOR);	//	Hotspots/Gateways
	sprintf(disp_str, "%d", hDate->origdate[0]);
	sf_loadFont(NotoSansBold70);
	sf_setCursor(12,76);
	sf_setTextColor(TFT_WHITE,MBG_COLOR,false);
	sf_writeStr((const char*)disp_str,2,CENTER_ALIGNED);
	sf_loadFont(NotoSansBold12);
	
	sf_setTextColor(TFT_WHITE,MBG_COLOR,true);

	memset(disp_str, 0, 50);
	sprintf(disp_str, "%.6f", gDate->latitude / 10000000.0);
	sf_setCursor(TXT_X + 90, TXT_Y1);
	sf_writeStr(disp_str, 10 ,LEFT_ALIGNED);
	//Serial.printf("gDate->latitude = %.6f",gDate->latitude / 10000000.0);

	memset(disp_str, 0, 50);
	sprintf(disp_str, "%.6f", gDate->longitude/ 10000000.0);
	sf_setCursor(TXT_X + 90, TXT_Y2);
	sf_writeStr(disp_str, 10 ,LEFT_ALIGNED);
	//Serial.printf("gDate->longitude = %.6f",gDate->longitude / 10000000.0);

	memset(disp_str, 0, 50);
	sprintf(disp_str, "%.2fm", (float)gDate->altitude / 1000.0);
	sf_setCursor(TXT_X + 90, TXT_Y3);
	sf_writeStr(disp_str, 10 ,LEFT_ALIGNED);
	//Serial.printf("gDate->altitude = %.2fm",(float)gDate->altitude / 1000.0);

	memset(disp_str, 0, 50);
	sprintf(disp_str, "%.1f", (float)gDate->hdop / 100);
	sf_setCursor(TXT_X + 90, TXT_Y4);
	sf_writeStr(disp_str, 10 ,LEFT_ALIGNED);
	//Serial.printf("gDate->hdop = %.1f",(float)gDate->hdop / 100);

	memset(disp_str, 0, 50);
	sprintf(disp_str, "%d", gDate->sats );
	sf_setCursor(TXT_X + 90, TXT_Y5);
	sf_writeStr(disp_str, 10 ,LEFT_ALIGNED);
	//Serial.printf("gDate->sats = %d",gDate->sats);
}

void gpsPageRefresh(hotspotsDate_t *hDate ,gpsDate_t* gDate)
{
	if(gDate->diplayState == true)
	{
		gDate->diplayState = false;
		gpsPageUpdate(hDate , gDate);
	}
}

void getCurrParamLimit(uint8_t band)
{
                     // EU433 CN470 RU864 IN865 EU868 US915 AU915 KR920 AS923-1 AS923-2 AS923-3 AS923-4	
	uint8_t minDr[12] 	= {0,			0,		0,		0,		0,		0,		2,		0,		2,				2,			2,			2,};
	uint8_t maxDr[12] 	= {5,			5,		5,		5,		5,		4,		6,		5,		5,				5,			5,			5,};
	
	uint8_t minTxp[12] 	= {0,			0,		0,		0,		0,		0,		0,		0,		0,				0,			0,			0,};
	uint8_t maxTxp[12] 	= {5,			7,		7,		10,		7,		14,		14,		7,		7,				7,			7,			7,};
	
	curr_dr_min		= minDr[band];
	curr_dr_max		= maxDr[band];
	curr_txp_min	=	minTxp[band];
	curr_txp_max	= maxTxp[band];
}

uint8_t coordinateX = 20;
uint8_t coordinateY = 65;

void settingPage()
{
	page = SETTINGS_PAGE;
	
	settingsArea = BAND_SET_AREA;

	drawBmp(&bmReturn35X28,RETURN_X,RETURN_Y);
	drawBmp(&bmUp31X31,UP_X,UP_Y);
	drawBmp(&bmDown31X31,DOWN_X,DOWN_Y);
	drawBmp(&bmRight31X31,RIGHT_X,RIGHT_Y);
	drawBmp(&bmOk53X29,OK_X,OK_Y);
	
	char region[12][10] = {"EU433","CN470","RU864","IN865","EU868","US915","AU915","KR920","AS923-1","AS923-2","AS923-3","AS923-4"};
	char dateRate[8][10] = {"DR0","DR1","DR2","DR3","DR4","DR5","DR6","DR7"};
	
	char disp_str[50] = {0};
	
	curr_band = (uint8_t)api.lorawan.band.get();
	curr_dr 	= (uint8_t)api.lorawan.dr.get();
	curr_txp 	= (uint8_t)api.lorawan.txp.get();
	
	
	last_band = curr_band;
	old_band 	= curr_band;
	last_dr 	= curr_dr;
	last_txp 	= curr_txp;

	uint8_t node_device_eui[8] = {0};
	uint8_t node_app_eui[8] = {0};
	uint8_t node_app_key[16] = {0};

	api.lorawan.deui.get(node_device_eui, 8);
	api.lorawan.appeui.get(node_app_eui, 8);
	api.lorawan.appkey.get(node_app_key, 16);
	
	sf_setTextColor(TFT_WHITE,MBG_COLOR,true);
	
	sf_setCursor(135, 50);
	sf_writeStr("SETTINGS", strlen("SETTINGS") ,LEFT_ALIGNED);
	
	sf_setCursor(BAND_STR_X, BAND_STR_Y);
	sf_writeStr("Band :", strlen("Band :") ,LEFT_ALIGNED);
	
	sf_setCursor(DR_STR_X, DR_STR_Y);
	sf_writeStr("DR :", strlen("DR :") ,LEFT_ALIGNED);
	
	sf_setCursor(TXP_STR_X, TXP_STR_Y);
	sf_writeStr("TX Power :", strlen("TX Power :") ,LEFT_ALIGNED);
	
	sf_setCursor(TXI_STR_X, TXI_STR_Y);
	sf_writeStr("TX Interval :", strlen("TX Interval :") ,LEFT_ALIGNED);
	
	sf_setCursor(BL_STR_X, BL_STR_Y);
	sf_writeStr("BackLight :", strlen("BackLight :") ,LEFT_ALIGNED);
	
	// sf_setCursor(WM_STR_X, WM_STR_Y);
	// sf_writeStr("Working Mode :", strlen("Working Mode :") ,LEFT_ALIGNED);
	
	sf_setCursor(VER_STR_X, VER_STR_Y);
	sf_writeStr("Version :  v1.1.2", strlen("Version :  v1.1.2") ,LEFT_ALIGNED);
	
	sf_setCursor(DEVEUI_STR_X, DEVEUI_STR_Y);
	sf_writeStr("DevEUI :", strlen("DevEUI :") ,LEFT_ALIGNED);
	
	sf_setCursor(APPEUI_STR_X, APPEUI_STR_Y);
	sf_writeStr("AppEUI :", strlen("AppEUI :") ,LEFT_ALIGNED);
	
	sf_setCursor(APPKEY_STR_X, APPKEY_STR_Y);
	sf_writeStr("AppKEY :", strlen("AppKEY :") ,LEFT_ALIGNED);
	
	memset(disp_str, 0, 50);
	for(int i = 0 , p = 0; i < sizeof(node_device_eui) ; i++)
	{
		p += sprintf(disp_str+p, "%02X", node_device_eui[i]);
	}
  sf_setCursor(DEVEUI_X, DEVEUI_Y);
	
  sf_writeStr(disp_str, strlen(disp_str) ,LEFT_ALIGNED);
	
	memset(disp_str, 0, 50);
	for(int i = 0 , p = 0; i < sizeof(node_app_eui) ; i++)
	{
		p += sprintf(disp_str+p, "%02X", node_app_eui[i]);
	}
  sf_setCursor(APPEUI_X, APPEUI_Y);
  sf_writeStr(disp_str, strlen(disp_str) ,LEFT_ALIGNED);
		
	memset(disp_str, 0, 50);
	for(int i = 0 , p = 0; i < sizeof(node_app_key); i++)
	{
		if(g_netWorkMode == 0)
		{
			p += sprintf(disp_str+p, "%02X", node_app_key[i]); // ForGateway
		}
		// else if(g_netWorkMode == 1)
		// {
			// p += sprintf(disp_str+p, "xx");	// ForHotSpot
		// }
		//p += sprintf(disp_str+p, "%02X", node_app_key[i]); // ForGateway
	}
  sf_setCursor(APPKEY_X, APPKEY_Y);
  sf_writeStr(disp_str, strlen(disp_str) ,LEFT_ALIGNED);
	
	sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
	memset(disp_str, 0, 50);
	memcpy(disp_str,region[curr_band],strlen(region[curr_band]));
	sf_setCursor(BAND_X, BAND_Y);
	sf_writeStr(disp_str, strlen(disp_str) ,LEFT_ALIGNED);
	
	memset(disp_str, 0, 50);
	memcpy(disp_str,dateRate[curr_dr],strlen(dateRate[curr_dr]));
  sf_setCursor(DR_X, DR_Y);
  sf_writeStr(disp_str, strlen(disp_str) ,LEFT_ALIGNED);
	
	memset(disp_str, 0, 50);
	sprintf(disp_str, "%d", curr_txp);
  sf_setCursor(TXP_X, TXP_Y);
  sf_writeStr(disp_str, strlen(disp_str) ,LEFT_ALIGNED);
	
	uint32_t txInt = (uint32_t)g_lorawanCfg.txInterval/1000;
	memset(disp_str, 0, 50);
	sprintf(disp_str, "%ds", txInt);
  sf_setCursor(TXI_X, TXI_Y);
  sf_writeStr(disp_str, strlen(disp_str) ,LEFT_ALIGNED);
	
	memset(disp_str, 0, 50);
	sprintf(disp_str, "%d", g_lorawanCfg.backLight);
  sf_setCursor(BL_X, BL_Y);
  sf_writeStr(disp_str, strlen(disp_str) ,LEFT_ALIGNED);
	
	curr_bl 			= g_lorawanCfg.backLight;
	curr_txi			= txInt;
	flashCount 		= 0;
}

void settingPageRefresh()
{	
	char region[12][10] = {"EU433","CN470","RU864","IN865","EU868","US915","AU915","KR920","AS923-1","AS923-2","AS923-3","AS923-4"};
	char dateRate[8][10] = {"DR0","DR1","DR2","DR3","DR4","DR5","DR6","DR7"};
	
	char disp_str[50] = {0};
	
	if(flashCount == 0)
	{
		switch(settingsArea)
		{
			case BAND_SET_AREA:
			{
				// BAND
				sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
				memset(disp_str, 0, 50);
				memcpy(disp_str,region[curr_band],strlen(region[curr_band]));
				sf_setCursor(BAND_X, BAND_Y);
				sf_writeStr(disp_str, strlen(disp_str) ,LEFT_ALIGNED);
				break;
			}
			case DR_SET_AREA:
			{
				// DR
				sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
				memset(disp_str, 0, 50);
				memcpy(disp_str,dateRate[curr_dr],strlen(dateRate[curr_dr]));
				sf_setCursor(DR_X, DR_Y);
				sf_writeStr(disp_str, strlen(disp_str) ,LEFT_ALIGNED);
				break;
			}
			case TXP_SET_AREA:
			{
				// TXP
				sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
				memset(disp_str, 0, 50);
				sprintf(disp_str, "%d", curr_txp);
				sf_setCursor(TXP_X, TXP_Y);
				sf_writeStr(disp_str, strlen(disp_str) ,LEFT_ALIGNED);
				break;
			}
			case TXI_SET_AREA:
			{
				// TXI
				sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
				memset(disp_str, 0, 50);
				sprintf(disp_str, "%ds", curr_txi);
				sf_setCursor(TXI_X, TXI_Y);
				sf_writeStr(disp_str, strlen(disp_str) ,LEFT_ALIGNED);
				break;
			}
			case BL_SET_AREA:
			{
				// BL
				sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
				memset(disp_str, 0, 50);
				sprintf(disp_str, "%d", curr_bl);
				sf_setCursor(BL_X, BL_Y);
				sf_writeStr(disp_str, strlen(disp_str) ,LEFT_ALIGNED);
				break;
			}
			default:
				break;
		}
	}
	else if(flashCount == 3)
	{
		switch(settingsArea)
		{
			case BAND_SET_AREA:
			{
				// BAND
				sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
				sf_setCursor(BAND_X, BAND_Y);
				sf_writeStr("                ", strlen("                ") ,LEFT_ALIGNED);
				break;
			}
			case DR_SET_AREA:
			{
				// DR
				sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
				sf_setCursor(DR_X, DR_Y);
				sf_writeStr("        ", strlen("        ") ,LEFT_ALIGNED);
				break;
			}
			case TXP_SET_AREA:
			{
				// TXP
				sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
				sf_setCursor(TXP_X, TXP_Y);
				sf_writeStr("      ", strlen("      ") ,LEFT_ALIGNED);
				break;
			}
			case TXI_SET_AREA:
			{
				// TXI
				sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
				sf_setCursor(TXI_X, TXI_Y);
				sf_writeStr("            ", strlen("            ") ,LEFT_ALIGNED);
				break;
			}
			case BL_SET_AREA:
			{
				// BL
				sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
				sf_setCursor(BL_X, BL_Y);
				sf_writeStr("      ", strlen("      ") ,LEFT_ALIGNED);
				break;
			}
			default:
				break;
		}
	}
	flashCount++;
	if(flashCount == 6)
		flashCount = 0;
}

void popUpPage()
{
	page = POPUP_PAGE;
	tft.fillRect(200 , 190 , 120 , 50 , BBG_CLOR);
	drawBmp(&bmPopUp73X27,236,200);
}

void refreshPopUpPage(uint8_t date )
{
	if(date == 1)
	{
		drawBmp(&bmPopLock18X23,195,9);
	}
	else
	{
		tft.fillRect(195, 9  , 18 , 23,TBG_COLOR);	//	Hotspots/Gateways
	}
}

void settingPageHandling(uint8_t button)
{
	char region[12][10] = {"EU433","CN470","RU864","IN865","EU868","US915","AU915","KR920","AS923-1","AS923-2","AS923-3","AS923-4"};
	char dateRate[8][10] = {"DR0","DR1","DR2","DR3","DR4","DR5","DR6","DR7"};
	
	char disp_str[50] = {0};
	
	bool setChangeFlag 		= 0;
	bool bandChangeFlag 	= 0;
	bool drChangeFlag 		= 0;
	bool txpChangeFlag 		= 0;
	bool txiChangeFlag 		= 0;
	bool blChangeFlag 		= 0;
	
	getCurrParamLimit(curr_band);
	switch(button)
	{
		case UP_SET_AREA:
		{
			switch(settingsArea)
			{
				case BAND_SET_AREA:
				{
					if(curr_band == 11)
						curr_band = 2;
					else
						curr_band++;
					bandChangeFlag = 1;
					getCurrParamLimit(curr_band);
					
					curr_dr 	= curr_dr_min;
					curr_txp 	= curr_txp_min;
					break;
				}
				case DR_SET_AREA:
				{
					if(curr_dr == curr_dr_max)
						curr_dr = curr_dr_min;
					else
						curr_dr ++;
					drChangeFlag = 1;

					break;
				}
				case TXP_SET_AREA:
				{
					if(curr_txp == curr_txp_max)
						curr_txp = curr_txp_min;
					else
						curr_txp ++;
					txpChangeFlag = 1;
					break;
				}
				case TXI_SET_AREA:
				{
					// if(g_netWorkMode == 1)
					// {
						// if(curr_txi == 3600)
							// curr_txi = 30;
						// else
							// curr_txi ++;
					// }
					// else
					// {
						if(curr_txi == 3600)
							curr_txi = 6;
						else
							curr_txi ++;
					// }
					txiChangeFlag = 1;
					break;
				}
				case BL_SET_AREA:
				{
					if(curr_bl == 10)
						curr_bl = 0;
					else
						curr_bl ++;
					blChangeFlag = 1;
					break;
				}
				default:
					break;
			}
			break;
		}
		case DOWN_SET_AREA: 
		{
			switch(settingsArea)
			{
				case BAND_SET_AREA:
				{
					if(curr_band == 2)
						curr_band = 11;
					else
						curr_band --;
					bandChangeFlag = 1;
					getCurrParamLimit(curr_band);
					
					curr_dr 	= curr_dr_min;
					curr_txp 	= curr_txp_min;
					break;
				}
				case DR_SET_AREA:
				{
					if(curr_dr == curr_dr_min)
						curr_dr = curr_dr_max;
					else
						curr_dr --;
					drChangeFlag = 1;
					break;
				}
				case TXP_SET_AREA:
				{
					if(curr_txp == curr_txp_min)
						curr_txp = curr_txp_max;
					else
						curr_txp --;
					txpChangeFlag = 1;
					break;
				}
				case TXI_SET_AREA:
				{
					// if(g_netWorkMode == 1)
					// {
						// if(curr_txi == 30)
							// curr_txi = 3600;
						// else
							// curr_txi --;
					// }
					// else
					// {
						if(curr_txi == 6)
							curr_txi = 3600;
						else
							curr_txi --;
					// }
					txiChangeFlag = 1;
					break;
				}
				case BL_SET_AREA:
				{
					if(curr_bl == 0)
						curr_bl = 10;
					else
						curr_bl --;
					blChangeFlag = 1;
					break;
				}
				default:
					break;
			}
			break;
		}
		case RIGHT_SET_AREA: 
		{
			if(settingsArea == BL_SET_AREA)
				settingsArea = BAND_SET_AREA;
			else
				settingsArea++;
			setChangeFlag = 1;
			break;
		}
		case OK_SET_AREA: 
		{
			// Save Cfg.
			g_lorawanCfg.backLight = curr_bl;
			g_lorawanCfg.txInterval = (uint32_t)(curr_txi)*1000;
			save_at_setting(SEND_FREQ_OFFSET);
			save_at_setting(BACK_LIGHYT_OFFSET);
			
			if(last_band != curr_band)
			{
				api.lorawan.band.set(curr_band);
				if(api.lorawan.band.get() == curr_band)
				{
					setBandResult = 1;
					last_band = curr_band;
				}
				else
					setBandResult = 0;
				delay(10);
			}
			else
			{
				setBandResult = 1;
			}
			
			if(last_dr != curr_dr)
			{
				api.lorawan.dr.set(curr_dr);
				if(api.lorawan.dr.get() == curr_dr)
				{
					setDrResult = 1;
					last_dr = curr_dr;
				}
				else
					setDrResult = 0;
				delay(10);
			}
			else
			{
				setDrResult = 1;
			}
			
			if(last_txp != curr_txp)
			{
				api.lorawan.txp.set(curr_txp);
				if(api.lorawan.txp.get() == curr_txp)
				{
					setTxpResult = 1;
					last_txp = curr_txp;
				}
				else
					setTxpResult = 0;
				delay(10);
			}
			else
			{
				setTxpResult = 1;
			}
			
			break;
		}
		default:
			break;
	}
	
	if(bandChangeFlag == 1)
	{
		sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
		// BAND
		memset(disp_str, 0, 50);
		memcpy(disp_str,region[curr_band],strlen(region[curr_band]));
		sf_setCursor(BAND_X, BAND_Y);
		sf_writeStr(disp_str, 8 ,LEFT_ALIGNED);
		// DR
		memset(disp_str, 0, 50);
		memcpy(disp_str,dateRate[curr_dr],strlen(dateRate[curr_dr]));
		sf_setCursor(DR_X, DR_Y);
		sf_writeStr(disp_str, 4 ,LEFT_ALIGNED);
		// TXP
		memset(disp_str, 0, 50);
		sprintf(disp_str, "%d", curr_txp);
		sf_setCursor(TXP_X, TXP_Y);
		sf_writeStr(disp_str, 3 ,LEFT_ALIGNED);
	}
	// DR
	if(drChangeFlag == 1)
	{
		sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
		memset(disp_str, 0, 50);
		memcpy(disp_str,dateRate[curr_dr],strlen(dateRate[curr_dr]));
		sf_setCursor(DR_X, DR_Y);
		sf_writeStr(disp_str, 4 ,LEFT_ALIGNED);
	}
	// TXP
	if(txpChangeFlag == 1)
	{
		sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
		memset(disp_str, 0, 50);
		sprintf(disp_str, "%d", curr_txp);
		sf_setCursor(TXP_X, TXP_Y);
		sf_writeStr(disp_str, 3 ,LEFT_ALIGNED);
	}
	// TXI
	if(txiChangeFlag == 1)
	{
		sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
		memset(disp_str, 0, 50);
		sprintf(disp_str, "%ds", curr_txi);
		sf_setCursor(TXI_X, TXI_Y);
		sf_writeStr(disp_str, 7 ,LEFT_ALIGNED);
	}
	// BL
	if(blChangeFlag == 1)
	{
		sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
		memset(disp_str, 0, 50);
		sprintf(disp_str, "%d", curr_bl);
		sf_setCursor(BL_X, BL_Y);
		sf_writeStr(disp_str, 4 ,LEFT_ALIGNED);
		setBackLight(curr_bl);
	}
	if(setChangeFlag == 1)
	{
		sf_setTextColor(ST77XX_GREEN,MBG_COLOR,true);
		// BAND
		memset(disp_str, 0, 50);
		memcpy(disp_str,region[curr_band],strlen(region[curr_band]));
		sf_setCursor(BAND_X, BAND_Y);
		sf_writeStr(disp_str, 8 ,LEFT_ALIGNED);
		// DR
		memset(disp_str, 0, 50);
		memcpy(disp_str,dateRate[curr_dr],strlen(dateRate[curr_dr]));
		sf_setCursor(DR_X, DR_Y);
		sf_writeStr(disp_str, 4 ,LEFT_ALIGNED);
		// TXP
		memset(disp_str, 0, 50);
		sprintf(disp_str, "%d", curr_txp);
		sf_setCursor(TXP_X, TXP_Y);
		sf_writeStr(disp_str, 3 ,LEFT_ALIGNED);
		// TXI
		memset(disp_str, 0, 50);
		sprintf(disp_str, "%ds", curr_txi);
		sf_setCursor(TXI_X, TXI_Y);
		sf_writeStr(disp_str, 7 ,LEFT_ALIGNED);
		// BL
		memset(disp_str, 0, 50);
		sprintf(disp_str, "%d", curr_bl);
		sf_setCursor(BL_X, BL_Y);
		sf_writeStr(disp_str, 4 ,LEFT_ALIGNED);
		setBackLight(curr_bl);
	}
}

uint8_t getSetResult()
{
	if((setBandResult == 0)|| (setDrResult == 0)||(setTxpResult == 0))
		return false;
	else
		return true;
}

uint8_t getBandChangeStatus()
{
	if( old_band 	!= curr_band )
	{
		old_band = curr_band;
		return true;
	}
	else
		return false;
}

void restartPage()
{
	page = RESTART_PAGE;
	tft.fillRect(0 , 40 , 320 , 147 , MBG_COLOR);
	tft.fillRect(0, 187 , 320 , 53  , MBG_COLOR);
	drawBmp(&bmRestartPop176X101,RESTART_POP_X,RESTART_POP_Y);
}

uint8_t lastDr;
uint8_t lastCfm;

void discoveryPage()
{
	// The discovery mode send updlink message with no ack to get the maximum of hotspot capturing the information. 

                     // EU433 CN470 RU864 IN865 EU868 US915 AU915 KR920 AS923-1 AS923-2 AS923-3 AS923-4	
//	uint8_t minDr[12] 	= {0,			0,		0,		0,		0,		0,		2,		0,		2,				2,			2,			2,};
//	uint8_t maxDr[12] 	= {5,			5,		5,		5,		5,		4,		6,		5,		5,				5,			5,			5,};
  uint8_t      sf[12] 	= {2,			2,		2,		2,		2,		0,		2,		2,		2,				2,			2,			2,}; // SF10
  uint8_t      band 	  = 0;
  
	page = DISCOVERY_PAGE;
	drawBmp(&bmReturn35X28,RETURN_X,RETURN_Y);
	g_discoveryMode = 1;
  band    = api.lorawan.band.get();
  lastDr  = api.lorawan.dr.get();
  lastCfm = api.lorawan.cfm.get();
  
	api.lorawan.cfm.set(0);	// No Ack.
	api.lorawan.dr.set(sf[band]);	// SF10.

  tft.fillRoundRect(42,85,236,30,3,tft.color565(49,49,49));
}

void showQRCode()
{
	uint8_t node_device_eui[8] = {0}; // ac1f09fff8683172
	if (api.lorawan.deui.get(node_device_eui, 8))
	{
		// LORA_LOG("Got DevEUI %02X%02X%02X%02X%02X%02X%02X%02X",
				// node_device_eui[0], node_device_eui[1], node_device_eui[2], node_device_eui[3],
				// node_device_eui[4], node_device_eui[5], node_device_eui[6], node_device_eui[7]);
	}
  // Create the QR code
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
	char url[128];
	sprintf(url,"https://dev.disk91.com/ft/r/?x=%02X%02X%02X%02X%02X", node_device_eui[3],node_device_eui[4], node_device_eui[5], node_device_eui[6], node_device_eui[7]);
  qrcode_initText(&qrcode, qrcodeData, 3, 0, url);
    
  int sz = 	20+qrcode.size*4;
  int xs =  160 - sz / 2;
  int ys =  110 - sz / 2;

  tft.fillRect(0 , 40 , 320 , 147 , MBG_COLOR);
  tft.fillRect(xs,ys,sz,sz,MBG_COLOR);
  for (uint8_t y = 0; y < qrcode.size; y++) 
  {
    for (uint8_t x = 0; x < qrcode.size; x++) 
    {
      if ( qrcode_getModule(&qrcode, x, y) ) 
      {
        tft.fillRect(10+xs+(4*x),10+ys+(4*y),4,4,TFT_WHITE);
      } 
      else 
      {
        tft.fillRect(10+xs+(4*x),10+ys+(4*y),4,4,MBG_COLOR);
      }
    }
  }
}

void discoveryPageRefresh()
{
  static 	uint8_t lastStatus = 255; // 0: NONE  1: LoraJoinFailed 2: GPSFixFailed

  char disp_str[50];
// tft.color565(154,255,149);// Green
// tft.color565(0,106,198);  // Blue

  if((discoveryStatus.status == 1) && (lastStatus == 1))
  {
    if(api.lorawan.njs.get() == true)
    {
      discoveryStatus.update = true;
      if(gpsDate.txState == true)
      {
        discoveryStatus.status = 0;
      }
      else
      {
        discoveryStatus.status = 2;
      }
    }
//    else
//    {
//      return;
//    }
  }

  if(lastStatus!=discoveryStatus.status)
  {
    lastStatus = discoveryStatus.status;
  }
  
  if(discoveryStatus.update == true)
  {
    discoveryStatus.update = false;
    if(discoveryStatus.sendCount == 0)
    {
      //discoveryStatus.update = true;
      tft.fillRect(166,98,3,3,tft.color565(0,106,198));
      tft.fillRect(158,98,3,3,tft.color565(0,106,198));
      tft.fillRect(150,98,3,3,tft.color565(0,106,198));
      tft.fillRect(45, 125  , 230 , 20,MBG_COLOR);
      memset(disp_str, 0, 15);
      sf_setCursor(90,130);
      sf_setTextColor(tft.color565(0,106,198),MBG_COLOR,false);
      if(discoveryStatus.status == 1)
      {
        sprintf(disp_str, "%d/10 Joining", discoveryStatus.sendCount);
      }
      else if(discoveryStatus.status == 2)
      {
        sprintf(disp_str, "%d/10 GPS Positioning", discoveryStatus.sendCount);
      }
      else
      {
        tft.fillRoundRect(42,85,236,30,3,tft.color565(49,49,49));
        sf_setTextColor(tft.color565(154,255,149),MBG_COLOR,false);
        sprintf(disp_str, "%d/10 Scanning", discoveryStatus.sendCount);
      }
      sf_writeStr((const char*)disp_str,22,CENTER_ALIGNED);
      return;
    }
    if((discoveryStatus.sendCount == 10)&&(discoveryStatus.status != 3))
    {
      discoveryStatus.status = 3;
      delay(100);
      showQRCode();
      return;
    }
    if(discoveryStatus.sendCount < 10)
    {
      if(discoveryStatus.status == 0)
      {
        tft.fillRoundRect(42,85,236,30,3,tft.color565(49,49,49));
        tft.fillRoundRect(45,88,discoveryStatus.sendCount*23,24,2,tft.color565(154,255,149));
        tft.fillRect(45, 125  , 230 , 20,MBG_COLOR);
        memset(disp_str, 0, 15);
        sprintf(disp_str, "%d/10 Scanning", discoveryStatus.sendCount);
        sf_setCursor(90,130);
        sf_setTextColor(tft.color565(154,255,149),MBG_COLOR,false);
        sf_writeStr((const char*)disp_str,22,CENTER_ALIGNED);
      }
      else if(discoveryStatus.status == 2)
      {
        tft.fillRoundRect(42,85,236,30,3,tft.color565(49,49,49));
        tft.fillRoundRect(45,88,discoveryStatus.sendCount*23,24,2,tft.color565(0,106,198));
        tft.fillRect(45, 125  , 230 , 20,MBG_COLOR);
        memset(disp_str, 0, 15);
        sprintf(disp_str, "%d/10 GPS Positioning", discoveryStatus.sendCount);
        sf_setCursor(90,130);
        sf_setTextColor(tft.color565(0,106,198),MBG_COLOR,false);
        sf_writeStr((const char*)disp_str,22,CENTER_ALIGNED);
      }
      return;
    }
  }
}

void quitDiscoveryPage()
{
  quitSubPage();
  api.lorawan.cfm.set(lastCfm);
  api.lorawan.dr.set(lastDr);
  g_discoveryMode = 0;
  discoveryStatus.status    = 0;
  discoveryStatus.sendCount = 0;
  discoveryStatus.update    = false;
}

static uint8_t lastPage = 0;

void shutDownPage()
{
  if(lastPage != SHUTDOWN_PAGE)
  {
    lastPage = page;
  }
  
	page = SHUTDOWN_PAGE;
  
  drawBmp(&bmReturn35X28   , RETURN_X,RETURN_Y);
	//drawBmp(&bmRestart66X66  , RESTART_X,RESTART_Y);
  drawBmp(&bmShutDown71X71 , SHUTDOWN_X,SHUTDOWN_Y);
}
void quitShutDownPage()
{
  switch(lastPage)
  {
    case HOME_PAGE:
    {
      subHomePage();
      break;
    }
    case HOTSPOTS_PAGE:
    {    
      hotSpotsPage(&hotspotsDate);
      break;
    }
    case RSSI_PAGE:
    {   
      rssiPage(&hotspotsDate , &rssiDate);
      break;
    }
    case DISTANCE_PAGE:
    {    
      distancePage(&hotspotsDate , &distanceDate);
      break;
    }
    case SNR_PAGE:
    {
      snrPage(&hotspotsDate , &snrDate);
      break;
    }
    case GPS_PAGE:
    {
      gpsPage(&hotspotsDate , &gpsDate);
      break;
    }
    case SETTINGS_PAGE:
    {
      settingPage();
      break;
    }
    case DISCOVERY_PAGE:
    {
      discoveryPage();
      discoveryStatus.update = true; 
      break;
    }
    default:
    break;
  }
}


static void tftFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  tft.startWrite();
  for (int16_t i = x; i < x + w; i++) 
  {
    tft.drawFastVLine(i, y, h, color);
  }
  tft.endWrite();
}

void FPSTest(void)
{
  float Ftime, Ltime;
  float FPS;
  Ftime = millis();
  int i;
  for(i = 100; i > 0; i--)
  {
    tftFillRect(0,0,240,320,TFT_WHITE);
    tftFillRect(0,0,240,320,TFT_BLACK);
  }
  Ltime = millis() - Ftime;
  FPS = 100.0f / (Ltime / 1000.0f) * 2.0f;
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);
  tft.print(Ltime);
  tft.setCursor(45, 0);
  tft.print("ms");
  tft.setCursor(0, 9);
  tft.print("FPS:");
  tft.setCursor(25, 9);
  tft.print(FPS);
}


void RAK14014_DeInit()
{
	tft.fillRect(0 , 0 , 320, 240, TFT_BLACK);
}

#define BUFFPIXEL 20
static void bmpDraw(const unsigned char *arry, uint8_t x, uint8_t y) 
{
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0;
	//uint32_t startTime = millis(),consumeTime = 0 ;

  if((x >= tft.width()) || (y >= tft.height())) 
  {
    return;
  }
  arrayPosition = 0;
  //Serial.println();

  // Parse BMP header
  if(read16FormArray(arry) == 0x4D42) 
  { // BMP signature
//    Serial.print("File size: "); 
//    Serial.println(read32FormArray(arry));
    read32FormArray(arry);
    (void)read32FormArray(arry); // Read & ignore creator bytes
    bmpImageoffset = read32FormArray(arry); // Start of image data
//    Serial.print("Image Offset: "); Serial.println(bmpImageoffset, DEC);
//    // Read DIB header
//    Serial.print("Header size: "); Serial.println(read32FormArray(arry));
    read32FormArray(arry);
    bmpWidth  = read32FormArray(arry);
    bmpHeight = read32FormArray(arry);
    if(read16FormArray(arry) == 1) 
    { // # planes -- must be '1'
      bmpDepth = read16FormArray(arry); // bits per pixel
//      Serial.print("Bit Depth: "); 
//      Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32FormArray(arry) == 0)) 
      { // 0 = uncompressed
        goodBmp = true; // Supported BMP format -- proceed!
//        Serial.print("Image size: ");
//        Serial.print(bmpWidth);
//        Serial.print('x');
//        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) 
        {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  
          w = tft.width()  - x;
        if((y+h-1) >= tft.height()) 
          h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.startWrite();
        tft.setAddrWindow(x, y, w, h);

        for (row=0; row<h; row++) 
        { // For each scanline...
          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(arrayPosition != pos) 
          { // Need seek?
            tft.endWrite();
            arrayPosition = pos;
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) 
          { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) 
            {
              arrayPosition += sizeof(sdbuffer);
              buffidx = 0; // Set index to beginning
              //bmpFile.read(sdbuffer, );              buffidx = 0; // Set index to beginning
              tft.startWrite();
            }

            b = arry[arrayPosition - sizeof(sdbuffer) + buffidx++];
            g = arry[arrayPosition - sizeof(sdbuffer) + buffidx++];
            r = arry[arrayPosition - sizeof(sdbuffer) + buffidx++];
            tft.pushColor(tft.color565(r,g,b));
          }
        }
        tft.endWrite();
//        consumeTime = millis() - startTime;
//        Serial.print("Loaded in ");
//        Serial.print(consumeTime);
//        Serial.println(" ms");
      }
    }
  }
  if(!goodBmp)
  {
    Serial.println("BMP format not recognized.");
  }
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.
static uint16_t read16FormArray(const unsigned char* p) 
{
  uint16_t result;
  ((uint8_t *)&result)[0] = p[arrayPosition]; // LSB
  arrayPosition++; 
  ((uint8_t *)&result)[1] = p[arrayPosition]; // MSB
  arrayPosition++; 
  return result;
}

static uint32_t read32FormArray(const unsigned char* p) 
{
  uint32_t result;
  ((uint8_t *)&result)[0] = p[arrayPosition]; // LSB
  arrayPosition++; 
  ((uint8_t *)&result)[1] = p[arrayPosition];
  arrayPosition++; 
  ((uint8_t *)&result)[2] = p[arrayPosition];
  arrayPosition++; 
  ((uint8_t *)&result)[3] = p[arrayPosition]; // MSB
  arrayPosition++; 
  return result;
}

static void drawBmp(const GUI_BITMAP *bmp , uint16_t x, uint16_t y) 
{
  uint16_t color = bmp->date[0];
  uint32_t count = 0;
  uint64_t bufSize = bmp->xSize * bmp->ySize;
  tft.startWrite();
  tft.setAddrWindow(x, y, bmp->xSize, bmp->ySize);

  for ( uint64_t i = 0 ; i < bufSize ; i++ ) 
  {
    if(color == bmp->date[i])
    {
      count++;
    }
    else
    {
      tft.writeColor(color,count); 
      count = 1;
      color = bmp->date[i];
    }
  }
  tft.writeColor(color,count); 
  tft.endWrite();
}



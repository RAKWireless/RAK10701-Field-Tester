#include "../src/libraries/_FT6336U.h"
#include "../Inc/bmp.h"
#include "../Inc/tp.h"
#include "../Inc/tft.h"
#include "../Inc/custom_at.h"


static void keyIntHandle(void);

FT6336U ft6336u(RST_PIN, INT_PIN); 
static uint8_t intSattus = 0;
static uint16_t tpX,tpY;

initState_t tpInit(void)
{
  ft6336u.begin(); // No reset. Reset is reset when the screen is reset
  pinMode(INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), keyIntHandle, FALLING);

  // TFT_DEBUG("FT6336U Firmware Version: %d",ft6336u.read_firmware_id()); 
  // TFT_DEBUG("FT6336U Device Mode: %d",ft6336u.read_device_mode()); 

  if(ft6336u.read_device_type() == 0x02)
  {
		ft6336u.disable_face_dec_mode();
		ft6336u.write_time_period_enter_monitor(0);
    return NONE; 
  }
  return INIT_TP_FAILED;
}

uint8_t getHomeTouchArea()
{
  uint8_t area = ERROR_AREA;

  //tpY = 239 - ft6336u.read_touch1_x();
	tpY = ft6336u.read_touch1_x();
  tpX = 319 - ft6336u.read_touch1_y();
  TFT_LOG("tpX : %d, tpY : %d",tpX,tpY);

	if(((RSSI_X1 <= tpX) && (tpX <= RSSI_X2))&&((RSSI_Y1 <= tpY) && (tpY <= RSSI_Y2)))
  {
    area = RSSI_AREA;
  }
  else if(((SNR_X1 <= tpX) && (tpX <= SNR_X2))&&((SNR_Y1 <= tpY) && (tpY <= SNR_Y2)))
  {
    area = SNR_AREA;
  }
  else if(((DISTANCE_X1 <= tpX) && (tpX <= DISTANCE_X2))&&((DISTANCE_Y1 <= tpY) && (tpY <= DISTANCE_Y2)))
  {
    area = DISTANCE_AREA;
  }
  else if(((GPS_X1 <= tpX) && (tpX <= GPS_X2))&&((GPS_Y1 <= tpY) && (tpY <= GPS_Y2)))
  {
    area = GPS_AREA;
  }
  // else if(((SHUTDOWN_X1 <= tpX) && (tpX <= SHUTDOWN_X2))&&((SHUTDOWN_Y1 <= tpY) && (tpY <= SHUTDOWN_Y2)))
  // {
    // area = SHUTDOWN_AREA;
  // }
  else if(((HOTSPOTS_X1 <= tpX) && (tpX <= HOTSPOTS_X2))&&((HOTSPOTS_Y1 <= tpY) && (tpY <= HOTSPOTS_Y2)))
  {
    area = HOTSPOTS_AREA;
  }
  else if(((SETTINGS_X1 <= tpX) && (tpX <= SETTINGS_X2))&&((SETTINGS_Y1 <= tpY) && (tpY <= SETTINGS_Y2)))
  {
    area = SETTINGS_AREA;
  }
	// else if(((210 <= tpX) && (tpX <= 253))&&((188 <= tpY) && (tpY <= 240))&&(g_netWorkMode == 1))
	else if(((210 <= tpX) && (tpX <= 253))&&((188 <= tpY) && (tpY <= 240)))
  {
    area = DISCOVERY_AREA;
  }

  return area;
}

uint8_t getAppTouchArea()
{
  uint8_t area = ERROR_AREA;
	uint8_t pageNum = getPageNum();

  tpY = ft6336u.read_touch1_x();
  tpX = 319 - ft6336u.read_touch1_y();
	TFT_LOG("tpX : %d, tpY : %d",tpX,tpY);
	
	if(pageNum == HOTSPOTS_PAGE)
	{
		if((70 <= tpY) && (tpY <= 148))
		{
			if((40 <= tpX)&&(tpX < 70))
				area = DATE10_AREA;
			else if ((70 <= tpX)&&(tpX < 97))
				area = DATE9_AREA;
			else if ((97 <= tpX)&&(tpX < 123))
				area = DATE8_AREA;
			else if ((123 <= tpX)&&(tpX < 150))
				area = DATE7_AREA;
			else if ((150 <= tpX)&&(tpX < 176))
				area = DATE6_AREA;
			else if ((176 <= tpX)&&(tpX < 207))
				area = DATE5_AREA;
			else if ((207 <= tpX)&&(tpX < 233))
				area = DATE4_AREA;
			else if ((233 <= tpX)&&(tpX < 259))
				area = DATE3_AREA;
			else if ((259 <= tpX)&&(tpX < 284))
				area = DATE2_AREA;
			else if ((284 <= tpX)&&(tpX < 320))
				area = DATE1_AREA;
		}
		else if(((RETURN_X1 <= tpX) && (tpX <= RETURN_X2))&&((RETURN_Y1 <= tpY) && (tpY <= RETURN_Y2)))
		{
			area = RETURN_AREA;
		}
	}
	else if(pageNum== SETTINGS_PAGE)
	{
		if(((UP_X1 <= tpX) && (tpX <= UP_X2))&&((UP_Y1 <= tpY) && (tpY <= UP_Y2)))
		{
			area = UP_SET_AREA;
		}
		else if(((DOWN_X1 <= tpX) && (tpX <= DOWN_X2))&&((DOWN_Y1 <= tpY) && (tpY <= DOWN_Y2)))
		{
			area = DOWN_SET_AREA;
		}
		else if(((RIGHT_X1 <= tpX) && (tpX <= RIGHT_X2))&&((RIGHT_Y1 <= tpY) && (tpY <= RIGHT_Y2)))
		{
			area = RIGHT_SET_AREA;
		}
		else if(((OK_X1 <= tpX) && (tpX <= OK_X2))&&((OK_Y1 <= tpY) && (tpY <= OK_Y2)))
		{
			area = OK_SET_AREA;
		}
		else if(((RETURN_X1 <= tpX) && (tpX <= RETURN_X2))&&((RETURN_Y1 <= tpY) && (tpY <= RETURN_Y2)))
		{
			area = RETURN_AREA;
		}
	}
	else if(pageNum == SHUTDOWN_PAGE)
	{
		// if(((RESTART_X1 <= tpX) && (tpX <= RESTART_X2))&&((RESTART_Y1 <= tpY) && (tpY <= RESTART_Y2)))
		// {
			// area = RESTART_AREA;
		// }
		if(((SHUTDOWN_X1 <= tpX) && (tpX <= SHUTDOWN_X2))&&((SHUTDOWN_Y1 <= tpY) && (tpY <= SHUTDOWN_Y2)))
		{
			area = SHUTDOWN_AREA;
		}
		else if(((RETURN_X1 <= tpX) && (tpX <= RETURN_X2))&&((RETURN_Y1 <= tpY) && (tpY <= RETURN_Y2)))
		{
			area = RETURN_AREA;
		}

	}
	else if(pageNum == RESTART_PAGE)
	{
		if(((70 <= tpX) && (tpX <= 150))&&((150 <= tpY) && (tpY <= 185)))
		{
			TFT_LOG("BACK_AREA");
			area = BACK_AREA;
		}
		else if(((170 <= tpX) && (tpX <= 250))&&((150 <= tpY) && (tpY <= 185)))
		{
			TFT_LOG("RESTART_AREA");
			area = RESTART_AREA;
		}
	}
	else
	{
		if((70 <= tpY) && (tpY <= 148))
		{
			if((XC1_POSITION1 <= tpX)&&(tpX < XC1_POSITION2))
				area = DATE10_AREA;
			else if ((XC2_POSITION1 <= tpX)&&(tpX < XC2_POSITION2))
				area = DATE9_AREA;
			else if ((XC3_POSITION1 <= tpX)&&(tpX < XC3_POSITION2))
				area = DATE8_AREA;
			else if ((XC4_POSITION1 <= tpX)&&(tpX < XC4_POSITION2))
				area = DATE7_AREA;
			else if ((XC5_POSITION1 <= tpX)&&(tpX < XC5_POSITION2))
				area = DATE6_AREA;
			else if ((XC6_POSITION1 <= tpX)&&(tpX < XC6_POSITION2))
				area = DATE5_AREA;
			else if ((XC7_POSITION1 <= tpX)&&(tpX < XC7_POSITION2))
				area = DATE4_AREA;
			else if ((XC8_POSITION1 <= tpX)&&(tpX < XC8_POSITION2))
				area = DATE3_AREA;
			else if ((XC9_POSITION1 <= tpX)&&(tpX < XC9_POSITION2))
				area = DATE2_AREA;
			else if ((XC10_POSITION1 <= tpX)&&(tpX < XC10_POSITION2))
				area = DATE1_AREA;
		}
		else if(((RETURN_X1 <= tpX) && (tpX <= RETURN_X2))&&((RETURN_Y1 <= tpY) && (tpY <= RETURN_Y2)))
		{
			area = RETURN_AREA;
		}
	}
  
  return area;
}

uint8_t getTouchStatus()
{
  return intSattus;
}

static void keyIntHandle(void)
{
  static time_t lastTime = 0;

  if(millis() - lastTime > 300) // Keep pressing the touch screen interrupt pin will output a square wave of about 60HZ.
  {
    intSattus = 1;
  }
  lastTime  = millis();
  if(getPageNum() == SETTINGS_PAGE)
  {
    intSattus = 1;
  }
}

void clearTouchStatus()
{
  tpX = 0;
  tpY = 0;
  intSattus = 0;
}

uint8_t getTdStatus()
{
	return ft6336u.read_td_status();
}
/*
*/
void tpMonitorMode()
{
//	ft6336u.write_monitor_mode_period(0x14); // write invalid.
	/*
	 *0x00 : P_ACTIVE
	 *0x01 : P_MONITOR
	 *0x02 : P_STANDBY
	 *0x03 : P_HIBERNATE
	 */
	//ft6336u.write_power_mode(0x03);// Power AVG:87.7uA
	ft6336u.write_power_mode(0x01);// 
	
	//ft6336u.write_ctrl_mode(switch_to_monitor_mode);
}

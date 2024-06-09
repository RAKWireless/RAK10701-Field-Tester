/**
   @file  RAK10701-P-L.ino
   @author rakwireless.com
   @brief For RAK10701-P and RAK10701-L.
   @version 0.1
   @date 2023-8-2
   @copyright Copyright (c) 2023
**/

#include "main.h"

MillisTaskManager mtmMain;

static uint8_t  factoryTestFlag = 0; // For factroy test.
static uint32_t lasttxInterval  = 0;
static uint8_t  touchLockFlag   = 0;

void setup()
{
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  pinMode(BL_PIN, OUTPUT);
  digitalWrite(BL_PIN, LOW);
  // Initialize Serial for debug output.
  Serial.begin(115200);
  printTitle();

  if(initPeripheral() == INIT_FATAL_REE)
  {
    delay(5000);
    enterLowPowerMode(); // Critical circuit failure system shutdown.
  }

  if(factoryTestFlag == 0)
  {
    homePage(); // Show home page.
  
    mtmMain.Register(displayProcess, 100);  // Process display data every 100ms.               
    mtmMain.Register(gpsProcess, 5000);     // Update location information every 5s.
    mtmMain.Register(sendTask, g_lorawanCfg.txInterval);    
    mtmMain.Register(batProcess, 180000);   // 3min.
    mtmMain.Register(dfuDeal, 500);         // Not needed.
    //mtmMain.Register(dateDebug, 2000);    // Fake date.for test display.
  }
}

void loop()
{
  if(factoryTestFlag == 1)
  {
    enterfactoryTestMode(); // Factory test mode function.
  }
  
  mtmMain.Running(millis());
}

/*
 *@brief  Initialize peripherals function.
 */
initState_t initPeripheral()
{
  time_t timeout = millis();

  tftInit();
  bootPage(); // Show boot page.

  init_ver_at();
  init_networkMode_at();
  init_proDevInfo_at();
  get_at_setting(NETWORK_MODE_OFFSET);

  if(loraInit() != NONE)
  {
    refreshErroCode(INIT_LORA_FAILED);
    Serial.printf("[ERR %d]:LoRa initialization failed.\r\n",INIT_LORA_FAILED);
    return INIT_FATAL_REE;
  }

  if(g_netWorkMode == 2)
  {
    init_factoryTest_at();
    factoryTestFlag = 1;
    return NONE;
  }

  get_at_setting(BACK_LIGHYT_OFFSET);
  pwmInit();  // Backlight Control.
  pwmBacklightFades(10 , g_lorawanCfg.backLight); // Screen backlight gradually turns on.
  
  if(gpsInit() != NONE)
  {
    refreshErroCode(INIT_GPS_FAILED);
    Serial.printf("[ERR %d]:GPS initialization failed.\r\n",INIT_GPS_FAILED);
    return INIT_FATAL_REE;
  }
  buttonInit();
  if(tpInit() != NONE)
  {
    refreshErroCode(INIT_TP_FAILED);
    Serial.printf("[ERR %d]:TP initialization failed.\r\n",INIT_TP_FAILED);
    return INIT_FATAL_REE;
  }
  init_frequency_at();  // User AT commands.
  if(get_at_setting(SEND_FREQ_OFFSET) != true)
  {
    refreshErroCode(INIT_FLASH_FAILED);
    Serial.printf("[ERR %d]:Flash initialization failed.\r\n",INIT_FLASH_FAILED);
    delay(2000);
  }
  lasttxInterval = g_lorawanCfg.txInterval;
  
  uint8_t node_device_eui[8] = {0}; // ac1f09fff8683172
  api.lorawan.deui.get(node_device_eui, 8);
  api.ble.settings.txPower.set(4);
  char dev_name[15] = {0};
  sprintf(dev_name,"RAK10701.%02X%02X%02X", node_device_eui[5], node_device_eui[6], node_device_eui[7]);
  //api.ble.uart.start();
  api.ble.uart.start(0);
  api.ble.settings.broadcastName.set(dev_name, 15);
  api.ble.advertise.start(0);

  batInit();
  uint8_t batVal = batRead();
  
  if((batVal == 255) || (batVal == 0)) // The battery is too low to run the system anymore
  {
    refreshErroCode(INIT_BAT_REE);
    Serial.printf("[ERR %d]:Low battery.\r\n",INIT_BAT_REE);
    delay(5000);
    //return INIT_FATAL_REE;
  }
  while((millis() - timeout) < 4000);
  return NONE;
}

/*
 *@brief  LoRaWAN timing sending function.
 */
void gpsProcess()
{
  gpsRead();
  if(gpsDate.txState == false)
  {
    APP_LOG("GPS data acquisition failed.\r\n");
  }
}

/*
 *@brief  LoRaWAN timing sending function.
 */
static uint8_t freqChangeFlag = 0;
void sendTask()
{
  static uint8_t changeFlag = 0;
  
  if(g_discoveryMode == 1)
  {
    if(freqChangeFlag == 0)
    {
      freqChangeFlag = 1;
      mtmMain.SetIntervalTime(sendTask, 30000); // 40000
    }
    
    if(discoveryStatus.status != 3)
    {
      if(api.lorawan.njs.get() == true)
      {
        if(gpsDate.txState == true)
        {
          discoveryStatus.status = 0;
          discoveryStatus.update = true;
          if(discoveryStatus.sendCount!=10)
          {
            loraSend(&gpsDate);
            discoveryStatus.sendCount+=1;
          }
        }
        else
        {
          discoveryStatus.status = 2;
          discoveryStatus.update = true;
        }
      }
      else
      {
        discoveryStatus.update = true;
        discoveryStatus.status = 1;
        loraSend(&gpsDate);
      }
    }
  }
  if((gpsDate.txState == true) || (getLoraSatus() == LORA_JOIN_FAILED))
  {
//    if((g_lorawanCfg.txInterval != 0) || (getLoraSatus() == LORA_JOIN_FAILED) )
    loraSend(&gpsDate);
    g_displayFlag = 1;
  }
  else
  {
    g_displayFlag = 0; 
  }

  if(g_sendIntervalLimit == 1)
  {
    if(changeFlag == 0)
    {
      changeFlag = 1;
      mtmMain.SetIntervalTime(sendTask, (MAXNONMOVEMENT_DURATION_MS)); // 900000
      LORA_LOG("No Movement, Limit upload rate to 15 minutes.");
    }
  }
  if(g_sendIntervalLimit == 0)
  {
    if(changeFlag == 1)
    {
      changeFlag = 0;
      mtmMain.SetIntervalTime(sendTask, g_lorawanCfg.txInterval); 
      LORA_LOG("Start Movement.");
    }
  }
}

/*
 *@brief  Read and display battery level function.
 *        Use battery voltage to estimate battery charge.
 *@note   In the charging state, the battery voltage is in a fluctuating state, 
 *        so the calculated battery power has a large error.
 */
void batProcess()
{
  uint8_t batVal = batRead();
  refreshBAT(batVal); // Update battery power icon.
  if(batVal == 255) // The battery is too low to run the system anymore
  {
    Serial.printf("[ERR %d]:low battery.\r\n",INIT_BAT_REE);
    enterLowPowerMode();
  }
}
void dfuDeal()
{
  delay(5);  // Delay is used to process serial commands.
}

/*
 *@brief  Force uplink function.
 *        Even if GPS is not located, network data can still be obtained by forced upload, 
 *        including RSSI SNR and Gateways/Hotspots quantity.
 */
time_t forceUplinkDisplayTimeout = 0;
void forceUplink()
{
  if( g_forceUplinkFlag == 2 )
  {
    uint8_t loraStatus = getLoraSatus();
    if((loraStatus == LORA_IDLE) || (loraStatus == LORA_JOINED))
    {
      mtmMain.ReSetTaskTime(sendTask, millis());
      loraSend(&gpsDate);
      forceUplinkDisplayTimeout = millis();
    }
  }
  
  if((g_forceUplinkFlag == 2) && (forceUplinkDisplayTimeout != 0))
  {
    if((millis() - forceUplinkDisplayTimeout) > 4000) 
    {
      g_forceUplinkFlag = 0;
      forceUplinkDisplayTimeout = 0;
    } 
  }
  
  if(lasttxInterval != g_lorawanCfg.txInterval)
  {
    lasttxInterval = g_lorawanCfg.txInterval;
    mtmMain.SetIntervalTime(sendTask, g_lorawanCfg.txInterval); 
  }
}

/*
 * @brief  Page data update function.
 *         All interface data updates are processed here.
 */
void displayProcess()
{
  /*
   * Touch screen event response.
   */
  static time_t   screenOffTimer  = millis();
  uint8_t         usbStatus       = NRF_POWER->USBREGSTATUS;
  
  if(getTouchStatus()) // Whether the touch screen is triggered.
  {
    if(g_screenOffFlag == 1)
    {
      clearTouchStatus();
      return;
    }
    screenOffTimer = millis();

    switch(getPageNum())
    {
      case HOME_PAGE: // Home page event handling.
      {
        if(touchLockFlag == 0)  
        { 
          homePageHandling();
        }
        break;
      }
      case HOTSPOTS_PAGE: // HotSpots page event handling.
      case RSSI_PAGE:
      case DISTANCE_PAGE:
      case SNR_PAGE:
      case GPS_PAGE:
      case SETTINGS_PAGE:
      case RESTART_PAGE:
      {
        if(touchLockFlag == 0)  
        {
          subPageHandling();
        }
        break;
      }
      case DISCOVERY_PAGE:
      {
        if(touchLockFlag == 0)  
        {
          discoveryPageHandling();
        }
        break;
      }
      case SHUTDOWN_PAGE:
      {
        shutDownPageHandling();
        break;
      }
      default:
        break;
    }
    clearTouchStatus();
  }
  /*
   * The automatic screen break time is adjusted from 5 minutes to 3 minutes.
   * @note: The automatic screen break time is adjusted from 5 minutes to 3 minutes.
   */
  if(((millis() - screenOffTimer) >  180000) && (g_screenOffFlag != 1) && ( touchLockFlag !=1 )) // 300000
  {
    analogWrite(BL_PIN, 0);
    g_screenOffFlag = 1;
    APP_LOG("screenOff...\r\n");
    return;
  }
  /*
   * Update display data on the TFT interface.
   */
  uint8_t pageNum = getPageNum();
  switch(pageNum)
  {
    case HOME_PAGE:
    {
      // Refresh the data displayed on the main interface.
      if(getLoraRxDateStatus() == REFRESHED)
      {
        refreshHomePage(&gpsDate , &hotspotsDate, &distanceDate , &rssiDate , &snrDate);
        setLoraRxDateStatus();
      }
      if(getGpsStatus() == FIXED_OK)
      {
        refreshGpsDate(&gpsDate);
        setGpsStatus();  
      }

      break;
    }
    case HOTSPOTS_PAGE: // HotSpots page event handling.
    {
      hotSpotsPageRefresh(&hotspotsDate);
      break;
    }
    case RSSI_PAGE:
    {
      rssiPageRefresh(&hotspotsDate , &rssiDate);
      break;
    }
    case DISTANCE_PAGE:
    {
      distancePageRefresh(&hotspotsDate , &distanceDate);
      break;
    }
    case SNR_PAGE:
    {
      snrPageRefresh(&hotspotsDate , &snrDate);
      break;
    }
    case GPS_PAGE:
    {
      gpsPageRefresh(&hotspotsDate , &gpsDate);
      break;
    }
    case SETTINGS_PAGE:
    {
      settingPageRefresh();
      break;
    }
    case DISCOVERY_PAGE:
    {
      discoveryPageRefresh();
      break;
    }
    default:
      break;
  }

  if(usbStatus != 3)
  {
    touchLockFlag = 0;
  }
  refreshScreenLock(touchLockFlag , pageNum);

  switch(getButtonStatus())
  {
    case LONG_PRESS:
    {
      if((g_screenOffFlag == 0)&&(getPageNum()!=SHUTDOWN_PAGE))
      {
        APP_LOG("LONG_PRESS");
        quitSubPage();
        shutDownPage();
      }
      break;
    }
    case DOUBLE_CLICK:
    {
/*      if((g_screenOffFlag == 0)&&(getPageNum()!=SHUTDOWN_PAGE))
      {
        APP_LOG("DOUBLE_CLICK = %d", getPageNum());  //TODO: Sometimes touch interrupt is triggered.
        if(touchLockFlag == 0)
        {
          touchLockFlag = 1;
          refreshPopUpPage(touchLockFlag);
        }
        else if(touchLockFlag == 1)
        {
          touchLockFlag = 0;
          refreshPopUpPage(touchLockFlag);
        }
      }*/
      if(g_screenOffFlag == 0)
      {
       if(g_forceUplinkFlag == 0)
       {
         g_forceUplinkFlag = 1;
       }
      }
      break;
    }
    case SINGLE_CLICK:
    {
      if((usbStatus == 3) && (g_screenOffFlag == 0) && (getPageNum()!=SHUTDOWN_PAGE))
      {
        if(touchLockFlag == 0)
          touchLockFlag = 1;
        else if(touchLockFlag == 1)
          touchLockFlag = 0;
        screenOffTimer = millis();
        break;
      }
      if(getPageNum()!=SHUTDOWN_PAGE)
      {
        APP_LOG("SINGLE_CLICK"); 
        if(g_screenOffFlag == 0)
        {
          g_screenOffFlag = 1;
          analogWrite(BL_PIN, 0);
          APP_LOG("screenOff...\r\n");  
        }
        else if(g_screenOffFlag == 1)
        {
          g_screenOffFlag = 0;
          setBackLight(g_lorawanCfg.backLight);
          APP_LOG("screenOn...\r\n");  
        }
        screenOffTimer = millis();
      }
      break;
    }
    case BUTTONSTATE_NONE:
    default:
      break;
  }

  /*
   * GPS icon update.
   */
  refreshGps(gpsDate.state);
  
  /*
   * Lora status update.
   */
  uint8_t loraSatus = getLoraSatus();
  refreshStatus(loraSatus);
  
  /*
   * force Uplink.
   */
  forceUplink();

  /*
 * Lora send status update.
 */
  refreshSendStatus(getLoraSendSatus());

  /*
   * Charging icon update.
   */
  refreshCharge(usbStatus);
}

/*
 *@brief Fake data, used to verify the display effect of the interface.
 */
void dateDebug()
{
  addHotspotsDate(&hotspotsDate , random(0,30));
  addRssiDate(&rssiDate,random(-130,20),random(-130,20));
  addDistanceDate(&distanceDate,random(0,32000),random(0,32000));
  addSnrDate(&snrDate,random(-30,30));
  if(getPageNum() == HOME_PAGE)
  {
    refreshHomePage(&gpsDate ,&hotspotsDate, &distanceDate , &rssiDate , &snrDate);
  }
}

/*
 *@brief Home page handler function.
 *       Complete the scheduling from the main page to the sub-page.
 */
void homePageHandling(void)
{
  switch(getHomeTouchArea())
  {
    case HOTSPOTS_AREA:
    {
      quitSubPage();     
      hotSpotsPage(&hotspotsDate);
      APP_LOG("HOTSPOTS AREA.\r\n");
      break;
    }
    case RSSI_AREA:
    {
      quitSubPage();      
      rssiPage(&hotspotsDate , &rssiDate);
      APP_LOG("RSSI AREA.\r\n");
      break;
    }
    case DISTANCE_AREA:
    {
      quitSubPage();      
      distancePage(&hotspotsDate , &distanceDate);
      APP_LOG("DISTANCE AREA.\r\n");
      break;
    }
    case SNR_AREA:
    {
      quitSubPage();
      snrPage(&hotspotsDate , &snrDate);
      APP_LOG("SNR AREA.\r\n");
      break;
    }
    case GPS_AREA:
    {
      quitSubPage();
      gpsPage(&hotspotsDate , &gpsDate);
      APP_LOG("GPS AREA.\r\n");
      break;
    }
    case SETTINGS_AREA:
    {
      quitSubPage();
      settingPage();
      APP_LOG("SETTINGS AREA.\r\n");
      break;
    }
    case DISCOVERY_AREA:
    {
      quitSubPage();
      discoveryPage();
      mtmMain.SetIntervalTime(sendTask, 10); 
      APP_LOG("DISCOVERY AREA.\r\n");
      break;
    }
    default:
    break;
  }
}

/*
 *@brief Sub page handler function.
 *      Include:  Gateways/Hotspots page
 *                Rssi page
 *                Distance page
 *                Snr page
 */
void subPageHandling(void)
{
  uint8_t area = getAppTouchArea();
  if(area == RETURN_AREA)
  {
    quitSubPage();
    subHomePage();
    return;
  }
  else if((area == DATE1_AREA)||(area == DATE2_AREA)||(area == DATE3_AREA)||(area == DATE4_AREA)
        ||(area == DATE5_AREA)||(area == DATE6_AREA)||(area == DATE7_AREA)||(area == DATE8_AREA)
        ||(area == DATE9_AREA)||(area == DATE10_AREA)&& (getPageNum() != SETTINGS_PAGE) && (getPageNum() !=  RESTART_PAGE))
  {
    switch(getPageNum())
    {
      case HOTSPOTS_PAGE:
      {
        hotSpotsPageDisplayLineValue(&hotspotsDate,(area - DATE1_AREA));
        break;
      }
      case RSSI_PAGE:
      {
        rssiPageDisplayLineValue(&hotspotsDate , &rssiDate , (area - DATE1_AREA));
        break;
      }
      case DISTANCE_PAGE:
      {
        distancePageDisplayLineValue(&hotspotsDate , &distanceDate,(area - DATE1_AREA));
        break;
      }
      case SNR_PAGE:
      {
        snrPageDisplayLineValue(&hotspotsDate , &snrDate,(area - DATE1_AREA));
        break;
      }
      default:
        break;
    }
  }
  if(getPageNum() == SETTINGS_PAGE)
  {
    settingPageHandling(area);
    if((area == OK_SET_AREA) && (getSetResult()==true))
    {
      if(getBandChangeStatus())
      {
        restartPage();
      }
      else
      {
        quitSubPage();
        subHomePage();
        return; 
      } 
    }
  }
  if(getPageNum() ==  RESTART_PAGE)
  {
    if(area == BACK_AREA)  
    {
      quitSubPage();
      subHomePage();
      return;  
    }
    else if(area == RESTART_AREA)
    {
      NVIC_SystemReset();
    }
  }
}

/*
 *@brief Shutdown page handler function.
 */
void shutDownPageHandling()
{
  uint8_t area = getAppTouchArea();
  if(area == SHUTDOWN_AREA)
  {
    APP_LOG("shutdown...\r\n");
    enterLowPowerMode();
    return;
  }
//  else if(area == RESTART_AREA)
//  {
//    APP_LOG("reboot...\r\n");  
//    NVIC_SystemReset();
//  }
  else if(area == RETURN_AREA)
  {
    quitSubPage();
    quitShutDownPage(); 
  }
  return;
}

/*
 *@brief Discovery mode page handler function.
 *@Note  Discovery mode is only supported under the Helium network and using Paul's backend server.
 */
void discoveryPageHandling(void)
{
  uint8_t area = getAppTouchArea();
  if(area == RETURN_AREA)
  {
    freqChangeFlag = 0;
    quitDiscoveryPage();
    subHomePage();
    mtmMain.SetIntervalTime(sendTask, g_lorawanCfg.txInterval); 
    return;
  }
}

/*
 *@brief Enter the low-power function.
 *       Deinitialize all peripherals and set the button as the low-power wake-up source.
 */
void enterLowPowerMode(void)
{
  RAK14014_DeInit();

  api.ble.stop();
  //sd_ble_gap_adv_stop(); //   adv_stop();
  api.ble.advertise.stop();
  nrf_sdh_disable_request();
  while (nrf_sdh_is_enabled()) {}
  
  digitalWrite(WB_IO2, LOW); // Turn off the power.

  nrf_gpio_cfg_default(BL);
  nrf_gpio_cfg_default(CS);
  nrf_gpio_cfg_default(DC);
  SPI.end();
  nrf_gpio_cfg_default(WB_SPI_CLK);
  nrf_gpio_cfg_default(WB_SPI_MISO);
  nrf_gpio_cfg_default(WB_SPI_MOSI);

  delay(2000); // Enter low power consumption after a delay of 1s to prevent immediate startup after shutdown.
//  tpMonitorMode();
  Wire.end();
  nrf_gpio_cfg_default(WB_I2C1_SDA);
  nrf_gpio_cfg_default(WB_I2C1_SCL);

//  nrf_gpio_cfg_default(RST);

  nrf_gpio_cfg_default(INT_PIN);
  
  while(1)
  {    
    api.system.sleep.setup(RUI_WAKEUP_FALLING_EDGE, BUTTON_INT_PIN);
    api.system.sleep.all(); // Set to sleep forever.  Note: need Use RUI version > v4.0.0
    pinMode(BUTTON_INT_PIN, INPUT_PULLUP); 
    delay(3500); // Long press to turn off/on, the long press time is 5s.
    if(digitalRead(BUTTON_INT_PIN) == LOW)
    {
      NVIC_SystemReset();
    }
  }
}

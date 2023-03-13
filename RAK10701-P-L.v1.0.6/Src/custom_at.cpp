#include "../Inc/custom_at.h"

int freq_send_handler(SERIAL_PORT port, char *cmd, stParam *param);
int network_mode_handler(SERIAL_PORT port, char *cmd, stParam *param);
int set_AppKey_handler(SERIAL_PORT port, char *cmd, stParam *param);
int factory_test_handler(SERIAL_PORT port, char *cmd, stParam *param);
int ver_handler(SERIAL_PORT port, char *cmd, stParam *param);
int factory_bandSwitch_handler(SERIAL_PORT port, char *cmd, stParam *param);


// static void StrToHex(uint8_t *pbDest, const char *pbSrc, int nLen);

loraParam_t g_lorawanCfg = 
{
	.upDateFlag = false,
};

uint8_t g_netWorkMode = 0 ; // 0: GateWay Mode 1: Helium Mode 2:Factory Mode.

static uint8_t testMode = 255;

uint8_t  g_apps_KEY[16];

uint8_t  g_bandSwitch;

/**
 * @brief Add send-frequency AT command
 *
 * @return true if success
 * @return false if failed
 */
bool init_frequency_at(void)
{
	if(g_netWorkMode == 1)
	{
		return api.system.atMode.add((char *)"SENDFREQ",
									 (char *)"Set/Get the frequent automatic sending time values in seconds, 30~3600 seconds",
									 (char *)"SENDFREQ", freq_send_handler);
	}
	else
	{
		return api.system.atMode.add((char *)"SENDFREQ",
									 (char *)"Set/Get the frequent automatic sending time values in seconds, 6~3600 seconds",
									 (char *)"SENDFREQ", freq_send_handler);
	}
}

/**
 * @brief Add network mode selection AT command
 *
 * @return true if success
 * @return false if failed
 */
bool init_networkMode_at(void)
{
	return api.system.atMode.add((char *)"FACTORYNETWORKMODE",
								 (char *)"For production use, users should not operate this command",
								 (char *)"FACTORYNETWORKMODE", network_mode_handler);
}

/**
 * @brief Hotspot setAppKEY AT command
 *
 * @return true if success
 * @return false if failed
 */
bool init_setAppKEY_at(void)
{
	return api.system.atMode.add((char *)"FACTORYSETAPPKEY",
								 (char *)"For production use, users should not operate this command",
								 (char *)"FACTORYSETAPPKEY", set_AppKey_handler);
}

bool init_factoryTest_at(void)
{
	return api.system.atMode.add((char *)"TEST",
								 (char *)"For production use, users should not operate this command",
								 (char *)"TEST", factory_test_handler);
}

bool init_bandSwitch_at(void)
{
	return api.system.atMode.add((char *)"BANDSWITCH",
								 (char *)"For production use, users should not operate this command",
								 (char *)"BANDSWITCH", factory_bandSwitch_handler);
}


/**
 * @brief Add version AT command
 *
 * @return true if success
 * @return false if failed
 */
bool init_ver_at(void)
{
	return api.system.atMode.add((char *)"VERSION",
								 (char *)"get the version of the Field Tester for LoRaWAN firmware",
								 (char *)"VERSION", ver_handler);
}

/**
 * @brief Handler for send frequency AT commands
 *
 * @param port Serial port used
 * @param cmd char array with the received AT command
 * @param param char array with the received AT command parameters
 * @return int result of command parsing
 * 			AT_OK AT command & parameters valid
 * 			AT_PARAM_ERROR command or parameters invalid
 */
int freq_send_handler(SERIAL_PORT port, char *cmd, stParam *param)
{
	if (param->argc == 1 && !strcmp(param->argv[0], "?"))
	{
		Serial.print(cmd);
		Serial.printf("=%lds\r\n", g_lorawanCfg.txInterval / 1000);
	}
	else if (param->argc == 1)
	{
		//AT_LOG("param->argv[0] >> %s", param->argv[0]);
		for (int i = 0; i < strlen(param->argv[0]); i++)
		{
			if (!isdigit(*(param->argv[0] + i)))
			{
				AT_LOG("%d is no digit", i);
				return AT_PARAM_ERROR;
			}
		}

		uint32_t new_send_freq = strtoul(param->argv[0], NULL, 10);
		if(g_netWorkMode == 1)
		{
			if(new_send_freq < 30)
			{
				AT_LOG("The upload interval is too short, the minimum interval must be greater than 30s");
				return AT_PARAM_ERROR;
			}
		}
		else
		{
			if(new_send_freq < 6)
			{
				AT_LOG("The upload interval is too short, the minimum interval must be greater than 6s");
				return AT_PARAM_ERROR;
			}
		}
		if(g_netWorkMode == 1)
		{
			if(new_send_freq > 3600)
			{
				AT_LOG("The upload interval is too long, the maximum interval does not exceed 3600s");
				return AT_PARAM_ERROR;
			}
		}
		else
		{
			if(new_send_freq > 3600)
			{
				AT_LOG("The upload interval is too long, the maximum interval does not exceed 3600s");
				return AT_PARAM_ERROR;
			}
		}

		AT_LOG("Requested frequency %ld", new_send_freq);
		
		g_lorawanCfg.upDateFlag = true;

		g_lorawanCfg.txInterval = new_send_freq * 1000;

		AT_LOG("New frequency %ld", g_lorawanCfg.txInterval);
		save_at_setting(SEND_FREQ_OFFSET);
	}
	else
	{
		return AT_PARAM_ERROR;
	}

	return AT_OK;
}

int ver_handler(SERIAL_PORT port, char *cmd, stParam *param)
{
	if (param->argc == 1 && !strcmp(param->argv[0], "?"))
	{
		Serial.print(MODEL);
		Serial.println(VERSION); 
		Serial.printf("Build Time  %s  %s\r\n",__DATE__,__TIME__);
	}
	else
	{
		return AT_PARAM_ERROR;
	}
	return AT_OK;
}

int network_mode_handler(SERIAL_PORT port, char *cmd, stParam *param)
{
	if (param->argc == 1)
	{
		//AT_LOG("param->argv[0] >> %s", param->argv[0]);
		if(memcmp(param->argv[0],"ForHotspot",strlen("ForHotspot")) == 0)
		{
			g_netWorkMode = 1;
			AT_LOG("Hotspot Mode.");
		}
		else if(memcmp(param->argv[0],"ForGateway",strlen("ForGateway")) == 0)
		{
			g_netWorkMode = 0;
			AT_LOG("GateWay Mode.");
		}
		else if(memcmp(param->argv[0],"ForFactoryTest",strlen("ForFactoryTest")) == 0)
		{
			g_netWorkMode = 2;
			AT_LOG("Factory Test Mode.");
		}
		else
		{
			return AT_PARAM_ERROR;
		}
		save_at_setting(NETWORK_MODE_OFFSET);
		if(g_netWorkMode == 2)
		{
			api.lorawan.nwm.set(0);	
		}
		else 
		{
			api.lorawan.nwm.set(1);	
		}
	}
	else
	{
		return AT_PARAM_ERROR;
	}

	return AT_OK;
}

int factory_bandSwitch_handler(SERIAL_PORT port, char *cmd, stParam *param)
{
	if (param->argc == 1 && !strcmp(param->argv[0], "?"))
	{
		Serial.print(cmd);
		Serial.printf("=%d\r\n", g_bandSwitch);
	}
	else if (param->argc == 1)
	{
		//AT_LOG("param->argv[0] >> %s", param->argv[0]);
		for (int i = 0; i < strlen(param->argv[0]); i++)
		{
			if (!isdigit(*(param->argv[0] + i)))
			{
				AT_LOG("%d is no digit", i);
				return AT_PARAM_ERROR;
			}
		}

		uint32_t bandSwitch = strtoul(param->argv[0], NULL, 1);
		if((bandSwitch != 1) || (bandSwitch != 0))
		{
		  return AT_PARAM_ERROR;
		}
		save_at_setting(BAND_SWITCH_OFFSET);
	}
	else
	{
		return AT_PARAM_ERROR;
	}

	return AT_OK;
}



static void str2Hex(uint8_t *pbDest, const char *pbSrc, int nLen)
{
  char h1,h2;
  char s1,s2;
  int i;

  for (i=0; i<nLen/2; i++)
  {
    h1 = pbSrc[2*i];
    h2 = pbSrc[2*i+1];

    s1 = toupper(h1) - 0x30;
    if (s1 > 9)
        s1 -= 7;

    s2 = toupper(h2) - 0x30;
    if (s2 > 9)
        s2 -= 7;
    pbDest[i] = s1*16 + s2;
  }
} 

int set_AppKey_handler(SERIAL_PORT port, char *cmd, stParam *param)
{
	char date[32];
	if (param->argc == 1)
	{
		AT_LOG("param->argv[0] >> %s", param->argv[0]);
		if(memcmp(param->argv[0],"RSWVESGH",strlen("RSWVESGH")) == 0)
		{
			sscanf(param->argv[0],"RSWVESGH%s",date);
			str2Hex(g_apps_KEY,date,32);
			// for(int i = 0; i< 16 ; i++)
			// {
				// AT_LOG("g_apps_KEY[%d] >> %X", i,g_apps_KEY[i]);
			// }
		}
		else
		{
			return AT_PARAM_ERROR;
		}
		// else if(memcmp(param->argv[0],"ForGateway",strlen("ForGateway")) == 0)
		// {
			// g_netWorkMode = 0;
			// AT_LOG("GateWay Mode.");
		// }

		save_at_setting(APP_KEY_OFFSET);
	}
	else
	{
		return AT_PARAM_ERROR;
	}

	return AT_OK;
}

int factory_test_handler(SERIAL_PORT port, char *cmd, stParam *param)
{
	if (param->argc == 1)
	{
		//AT_LOG("param->argv[0] >> %s", param->argv[0]);
		if(memcmp(param->argv[0],"TFT_TP",strlen("TFT_TP")) == 0)
		{
			testMode = 0;
			AT_LOG("test TFT_TP");
		}
		else if(memcmp(param->argv[0],"LORA",strlen("LORA")) == 0)
		{
			testMode = 1;
			AT_LOG("test LORA");
		}
		else if(memcmp(param->argv[0],"BLE",strlen("BLE")) == 0)
		{
			testMode = 2;
			AT_LOG("test BLE");
		}
		else if(memcmp(param->argv[0],"GPS",strlen("GPS")) == 0)
		{
			testMode = 3;
			AT_LOG("test GPS");
		}
		else
		{
			return AT_PARAM_ERROR;
		}
	}
	else
	{
		return AT_PARAM_ERROR;
	}

	return AT_OK;
}
uint8_t get_test_mode()
{
	return testMode;
}
void set_test_mode()
{
	testMode = 255;
}

/**
 * @brief Get setting from flash
 *
 * @param setting_type type of setting, valid values
 * 			GNSS_OFFSET for GNSS precision and data format
 * @return true read from flash was successful
 * @return false read from flash failed or invalid settings type
 */
bool get_at_setting(uint32_t setting_type)
{
	uint8_t flash_value[18];
	switch (setting_type)
	{
	case SEND_FREQ_OFFSET:
		if (!api.system.flash.get(SEND_FREQ_OFFSET, flash_value, 5))
		{
			AT_LOG("Failed to read send frequency from Flash");
			if(g_netWorkMode == 1)
				g_lorawanCfg.txInterval = 30000;
			else
				g_lorawanCfg.txInterval = DEFAULT_TXINTERVAL;
			save_at_setting(SEND_FREQ_OFFSET);
			return false;
		}
		if (flash_value[4] != 0xAA)
		{
			AT_LOG("No valid send frequency found, set to default, read 0X%02X 0X%02X 0X%02X 0X%02X",
				  flash_value[0], flash_value[1],
				  flash_value[2], flash_value[3]);
			if(g_netWorkMode == 1)
				g_lorawanCfg.txInterval = 30000;
			else
				g_lorawanCfg.txInterval = DEFAULT_TXINTERVAL;
			save_at_setting(SEND_FREQ_OFFSET);
			return false;
		}

		AT_LOG("Read send frequency 0X%02X 0X%02X 0X%02X 0X%02X",
			  flash_value[0], flash_value[1],
			  flash_value[2], flash_value[3]);
		g_lorawanCfg.txInterval = 0;
		g_lorawanCfg.txInterval |= flash_value[0] << 0;
		g_lorawanCfg.txInterval |= flash_value[1] << 8;
		g_lorawanCfg.txInterval |= flash_value[2] << 16;
		g_lorawanCfg.txInterval |= flash_value[3] << 24;
		AT_LOG("Send frequency found %ld", g_lorawanCfg.txInterval);
		
		if(g_netWorkMode == 1)
		{
			if(g_lorawanCfg.txInterval<30000 || g_lorawanCfg.txInterval > 3600000)
			{
				g_lorawanCfg.txInterval = 30000;
				save_at_setting(SEND_FREQ_OFFSET);
				return false;
			}
		}
		
		if(g_netWorkMode == 0)
		{
			if(g_lorawanCfg.txInterval<6000 || g_lorawanCfg.txInterval > 3600000)
			{
				g_lorawanCfg.txInterval = 6000;
				save_at_setting(SEND_FREQ_OFFSET);
				return false;
			}
		}
		
		return true;
		break;
	case BACK_LIGHYT_OFFSET:
		if (!api.system.flash.get(BACK_LIGHYT_OFFSET, flash_value, 2))
		{
			AT_LOG("Failed to read TFT back light from Flash");
			g_lorawanCfg.backLight = DEFAULT_BACKLIGHT;
			return false;
		}
		if (flash_value[1] != 0xAA)
		{
			AT_LOG("No valid TFT back light, set to default, read 0X%02X 0X%02X",
				  flash_value[0], flash_value[1]);
			g_lorawanCfg.backLight = DEFAULT_BACKLIGHT;
			save_at_setting(BACK_LIGHYT_OFFSET);
			return false;
		}
		AT_LOG("Read TFT back light 0X%02X 0X%02X",
			  flash_value[0], flash_value[1]);
		g_lorawanCfg.backLight = flash_value[0];
		AT_LOG("TFT back light %d", g_lorawanCfg.backLight);
		return true;
		break;
		
	case NETWORK_MODE_OFFSET:
		if (!api.system.flash.get(NETWORK_MODE_OFFSET, flash_value, 2))
		{
			g_netWorkMode = 0;
			return false;
		}
		if (flash_value[1] != 0xAA)
		{
			g_netWorkMode = 0;
			save_at_setting(NETWORK_MODE_OFFSET);
			return false;
		}
		g_netWorkMode = flash_value[0];
		return true;
		break;
	case APP_KEY_OFFSET:
		if (!api.system.flash.get(APP_KEY_OFFSET, flash_value, 17))
		{
			for(int i = 0; i< 16 ; i++)
			{
				g_apps_KEY[i] = 0x00;
			}
			return false;
		}
		if (flash_value[16] != 0xAA)
		{
			for(int i = 0; i< 16 ; i++)
			{
				g_apps_KEY[i] = 0x00;
			}
			save_at_setting(APP_KEY_OFFSET);
			return false;
		}
		for(int i = 0; i< 16 ; i++)
		{
			g_apps_KEY[i] = flash_value[i];
		}
		return true;
		break;
	case BAND_SWITCH_OFFSET:
		if (!api.system.flash.get(BAND_SWITCH_OFFSET, flash_value, 2))
		{
      g_bandSwitch = 1;
			save_at_setting(BAND_SWITCH_OFFSET);
			return false;
		}
		if (flash_value[1] != 0xAA)
		{
      g_bandSwitch = 1;
			save_at_setting(BAND_SWITCH_OFFSET);
			return false;
		}
    
		g_bandSwitch = flash_value[0];
				
		return true;
		break;

	default:
		return false;
	}
}

/**
 * @brief Save setting to flash
 *
 * @param setting_type type of setting, valid values
 * 			GNSS_OFFSET for GNSS precision and data format
 * @return true write to flash was successful
 * @return false write to flash failed or invalid settings type
 */
bool save_at_setting(uint32_t setting_type)
{
	uint8_t flash_value[18] = {0};
	bool wr_result = false;
	switch (setting_type)
	{
	case SEND_FREQ_OFFSET:
		flash_value[0] = (uint8_t)(g_lorawanCfg.txInterval >> 0);
		flash_value[1] = (uint8_t)(g_lorawanCfg.txInterval >> 8);
		flash_value[2] = (uint8_t)(g_lorawanCfg.txInterval >> 16);
		flash_value[3] = (uint8_t)(g_lorawanCfg.txInterval >> 24);
		flash_value[4] = 0xAA; // 
		AT_LOG("AT_CMD", "Writing send frequency 0X%02X 0X%02X 0X%02X 0X%02X ",
			  flash_value[0], flash_value[1],
			  flash_value[2], flash_value[3]);
		wr_result = api.system.flash.set(SEND_FREQ_OFFSET, flash_value, 5);
		// AT_LOG("AT_CMD", "Writing %s", wr_result ? "Success" : "Fail");
		wr_result = true;
		return wr_result;
		break;
	
	case BACK_LIGHYT_OFFSET:
		flash_value[0] = g_lorawanCfg.backLight ;
		flash_value[1] = 0xAA;
		AT_LOG("AT_CMD", "Writing TFT back light 0X%02X 0X%02X 0X%02X 0X%02X ",
			  flash_value[0], flash_value[1]);
		wr_result = api.system.flash.set(BACK_LIGHYT_OFFSET, flash_value, 2);
		wr_result = true;
		return wr_result;
		break;
	case NETWORK_MODE_OFFSET:
		flash_value[0] = g_netWorkMode ;
		flash_value[1] = 0xAA;
		wr_result = api.system.flash.set(NETWORK_MODE_OFFSET, flash_value, 2);
		wr_result = true;
		return wr_result;
		break;
	case APP_KEY_OFFSET:
		for(int i = 0; i< 16 ; i++)
		{
			flash_value[i] = g_apps_KEY[i] ;
		}
		flash_value[16] = 0xAA;
		wr_result = api.system.flash.set(APP_KEY_OFFSET, flash_value, 17);
		wr_result = true;
		return wr_result;
		break;

	case BAND_SWITCH_OFFSET:
		flash_value[0] = g_bandSwitch;
		flash_value[1] = 0xAA; // 
 		AT_LOG("AT_CMD", "Writing send frequency 0X%02X 0X%02X",flash_value[0], flash_value[1]);
		wr_result = api.system.flash.set(BAND_SWITCH_OFFSET, flash_value, 2);
		wr_result = true;
		return wr_result;
		break;

	default:
		return false;
		break;
	}
	return false;
}

// static void StrToHex(uint8_t *pbDest, const char *pbSrc, int nLen)
// {
  // char h1,h2;
  // char s1,s2;
  // int i;

  // for (i=0; i<nLen/2; i++)
  // {
    // h1 = pbSrc[2*i];
    // h2 = pbSrc[2*i+1];

    // s1 = toupper(h1) - 0x30;
    // if (s1 > 9)
        // s1 -= 7;

    // s2 = toupper(h2) - 0x30;
    // if (s2 > 9)
        // s2 -= 7;
    // pbDest[i] = s1*16 + s2;
  // }
// }

#ifndef __CUSTOM_AT_H__
#define __CUSTOM_AT_H__

#include "common.h"

/** Settings offset in flash */
#define GNSS_OFFSET 				0x00000000
#define SEND_FREQ_OFFSET 		0x00000002 
#define BACK_LIGHYT_OFFSET 	0x00000008 
#define NETWORK_MODE_OFFSET 0x0000000B 		
#define APP_KEY_OFFSET 			0x00000010 		
#define BAND_SWITCH_OFFSET 	0x00000030

/** The default automatic sending interval is 20s **/
#define DEFAULT_TXINTERVAL	20000

/** The default back light is 10 **/
#define DEFAULT_BACKLIGHT		10

#define NORMAL_MODE  0
#define HELIUM_MODE  1
#define FACTORY_MODE 2

#define ID1 (0x10000060)
#define ID2 (0x10000064)

/**
 * @The data structure saved in Flash cannot exceed 128 bytes@NRF52840.
 */
 typedef struct 
{
	uint8_t upDateFlag;
	uint32_t txInterval;
	uint8_t backLight;
}loraParam_t; 

extern loraParam_t g_lorawanCfg;

extern uint8_t g_netWorkMode;

extern uint8_t  g_apps_KEY[16];

extern uint8_t  g_bandSwitch;

uint8_t get_test_mode();
void set_test_mode();
bool init_frequency_at(void);
bool init_networkMode_at(void);
bool init_setAppKEY_at(void);
bool init_factoryTest_at(void);
bool init_ver_at(void);
bool init_bandSwitch_at(void);
bool init_proDevInfo_at(void);

bool get_at_setting(uint32_t setting_type);
bool save_at_setting(uint32_t setting_type);

#endif
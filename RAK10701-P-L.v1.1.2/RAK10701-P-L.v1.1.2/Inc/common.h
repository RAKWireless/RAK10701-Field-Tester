#ifndef __COMMON_H__
#define __COMMON_H__

#include <Arduino.h>

#define MODEL					"Field Tester"
#define VERSION     	"Version: v1.1.2"

#define HW_VERSION		"VC"
#define FW_VERSION		"V1.1.2"			
//#define PRO_MODEL			"RAK10701-L"
#define PRO_MODEL			"RAK10701-P"

#define APP_DEBUG   0

#define TFT_DEBUG   1
#define BAT_DEBUG   1
#define GPS_DEBUG   1
#define LORA_DEBUG  1
#define AT_DEBUG   	1

#if APP_DEBUG > 0
  #define APP_LOG(...)                        \
    {                                         \
      Serial.printf("[%s:",__FUNCTION__);     \
      Serial.printf("%d] ",__LINE__);          \
      Serial.printf(__VA_ARGS__);             \
      Serial.printf("\n");                    \
      delay(10);                              \
    }
#else
  #define APP_LOG(...)
#endif

#if TFT_DEBUG > 0
  #define TFT_LOG(...)      APP_LOG(__VA_ARGS__)
#else
  #define TFT_LOG(...)
#endif

#if BAT_DEBUG > 0
  #define BAT_LOG(...)      APP_LOG(__VA_ARGS__)
#else
  #define BAT_LOG(...)
#endif

#if GPS_DEBUG > 0
  #define GPS_LOG(...)      APP_LOG(__VA_ARGS__)
#else
  #define GPS_LOG(...)
#endif

#if AT_DEBUG > 0
  #define AT_LOG(...)      APP_LOG(__VA_ARGS__)
#else
  #define AT_LOG(...)
#endif

#if LORA_DEBUG > 0
  #define LORA_LOG(...)      APP_LOG(__VA_ARGS__)
#else
  #define LORA_LOG(...)
#endif


#define RECORD_LENGTH     10 

/*
 * @brief Init states.
 */
typedef enum 
{
  INIT_GPS_FAILED	         = 0,
  INIT_TP_FAILED,
  INIT_FLASH_FAILED,
  INIT_LORA_FAILED,
  INIT_FATAL_REE,
	INIT_BAT_REE,
  NONE,
}initState_t;

/*
 * @brief LoRa Status.
 */
typedef enum 
{
  LORA_JOINING	         = 0,
  LORA_JOINED,
  LORA_JOIN_FAILED,
  LORA_SEND,
  LORA_IDLE,
  LORA_NONE,
}loraStatus_t;

/*
 * @brief LoRa Send Status.
 */
typedef enum 
{
	SEND_FAILED = 0,
	SEND_OK,        
  SEND_NONE,
}loraSendStatus_t;

/*
 * @brief LoRa Rx Refresh Status.
 */
typedef enum 
{
	NOT_REFRESHED = 0,
	REFRESHED,        
}loraRxRefreshStatus_t;

/*
 * @brief Hotspots data struct.
 */
typedef struct
{
  bool 		updateState;	// Data update status FALSE: not updated, TRUE: updated.
	uint16_t origdate[RECORD_LENGTH]; // Original data[0~15].
  int16_t mapDate[RECORD_LENGTH];  // Data mapped to pixel coordinates[70,160].
  uint8_t count;
}hotspotsDate_t;

/*
 * @brief RSSI data struct.
 */
typedef struct
{
  bool 		updateState;	// Data update status FALSE: not updated, TRUE: updated.
  int16_t maxOrigdate[RECORD_LENGTH]; // MAX Original data[-150~0].
  int16_t maxMapDate[RECORD_LENGTH];  // Data mapped to pixel coordinates[70,160].
  int16_t minOrigdate[RECORD_LENGTH]; // MAX Original data[-150~0].
  int16_t minMapDate[RECORD_LENGTH];  // Data mapped to pixel coordinates[70,160].
  uint8_t count;
}rssiDate_t;

/*
 * @brief Distance data struct.
 */
typedef struct
{
  bool 		updateState;	// Data update status FALSE: not updated, TRUE: updated.
  int16_t maxOrigdate[RECORD_LENGTH]; // MAX Original data[250~32000].  0: 0 means invalid value
  int16_t maxMapDate[RECORD_LENGTH];  // Data mapped to pixel coordinates[70,160].
  int16_t minOrigdate[RECORD_LENGTH]; // MAX Original data[250~32000].
  int16_t minMapDate[RECORD_LENGTH];  // Data mapped to pixel coordinates[70,160].
  uint8_t count;
}distanceDate_t;

/*
 * @brief SNR data struct.
 */
typedef struct
{
  bool 		updateState;	// Data update status FALSE: not updated, TRUE: updated.
  int16_t origdate[RECORD_LENGTH]; // Original data[0~30].
  int16_t mapDate[RECORD_LENGTH];  // data mapped to pixel coordinates[70,160].
  uint8_t count;
}snrDate_t;

/*
 * @brief GPS data struct.
 */
#define FRAMESIZE  10
typedef struct
{
	uint8_t state;		
  bool diplayState;  // Indicates if GPS data is valid.
	bool txState;  // Indicates if GPS data is valid.
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  
	long 			speed;
  uint16_t  hdop;           // Hdop x 100
  int64_t   longitude;      // 1 / 10_000_000
  int64_t   latitude;       // 1 / 10_000_000
	
	int64_t   backupLongitude;      // 1 / 10_000_000
  int64_t   backupLatitude;       // 1 / 10_000_000
	
  int32_t   altitude;       // in meters
  uint8_t   sats;           // Number of sats used
  char *    lastNMEA;       // Last NMEA String received
  uint64_t  encodePosition; // Compact encoding of the current position
  
/* @brief The following Frame format are used: uplink format on port 1
 * Byte Usage
 * 0 - 5  GSP position see here for details. Decoding see below
 * 6 - 7  Altitude in meters + 1000m ( 1100 = 100m )
 * 8      HDOP * 10 (11 = 1.1)
 * 9      Sats in view
*/
  uint8_t   frameDate[FRAMESIZE];   // Frame format: https://github.com/disk91/WioLoRaWANFieldTester/blob/master/doc/DEVELOPMENT.md
}gpsDate_t;

#endif

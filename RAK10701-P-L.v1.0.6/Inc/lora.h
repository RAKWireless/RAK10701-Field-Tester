#ifndef __LORA_H__
#define __LORA_H__

#include "common.h"

#define SEND_PORT					1
#define CONFIRM_MESSAGE		1
#define RETRY_TIMES				2

#define MAXNONMOVEMENT_DURATION_MS		900000 // Fair use - downgrade period when the position of the device is unchanged.

/*
 * @brief Discovery Mode Status.
 */
typedef struct
{
	uint8_t sendCount; // Send data 0~10.
  uint8_t status;
  uint8_t update;
}discoveryStatus_t;

initState_t loraInit();
void loraSend(gpsDate_t* frame);
uint8_t getLoraSatus();
uint8_t getLoraSendSatus();
uint8_t getLoraRxDateStatus();
void 		setLoraRxDateStatus();

void addHotspotsDate(hotspotsDate_t *date , uint16_t orDate);
void addRssiDate(rssiDate_t *date , int16_t orMaxDate, int16_t orMinDate);
void addDistanceDate(distanceDate_t *date , int16_t orMaxDate, int16_t orMinDate);
void addSnrDate(snrDate_t *date , int16_t orDate);

extern hotspotsDate_t hotspotsDate;
extern rssiDate_t rssiDate;
extern distanceDate_t distanceDate;
extern snrDate_t snrDate;

extern uint8_t g_sendIntervalLimit;

extern uint8_t g_discoveryMode;
extern discoveryStatus_t discoveryStatus;

extern time_t  g_sendTime;

#endif
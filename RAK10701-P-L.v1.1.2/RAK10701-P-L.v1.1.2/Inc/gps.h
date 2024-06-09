#ifndef __GPS_H__
#define __GPS_H__

#include "common.h"

/*
 * @brief Gps Status.
 */
typedef enum
{
	FIXED_OK = 0,
	NONE_FIXED,
} gpsStatus_t;

initState_t gpsInit();
void gpsRead(void);
void gpsDeInit();
int gpsEstimateDistance();

uint8_t getGpsStatus();
void setGpsStatus();

extern gpsDate_t gpsDate;

extern int64_t g_backupLongitude;
extern int64_t g_backupLatitude;

#endif

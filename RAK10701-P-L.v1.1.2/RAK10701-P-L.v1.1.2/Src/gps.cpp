
#include <Wire.h> //Needed for I2C to GNSS
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
#include "../Inc/gps.h"

SFE_UBLOX_GNSS GNSS;

/** Gps Status Indication */
static uint8_t gpsStatus = NONE_FIXED;


gpsDate_t gpsDate = 
{
	.state = false,
	.diplayState = false,
	.txState = false,
};

int64_t   g_backupLongitude; 
int64_t   g_backupLatitude; 

initState_t gpsInit()
{  
  Wire.begin();

  GPS_LOG("GPS ZOE-M8Q (I2C) Init.");
  if (GNSS.begin() == false) //Connect to the u-blox module using Wire port
  {
    GPS_LOG("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing.");
    return INIT_GPS_FAILED;
  }

  GNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  // GNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
	GNSS.setNavigationFrequency(5);  //Produce two solutions per second
	GNSS.setAutoPVT(true, false);    //Tell the GNSS to "send" each solution and the lib not to update stale data implicitly

  return NONE;
}

/**
 * Compact encoding of the current position
 * The result is stored in the **output** uint64_t variable
 * the result is stored in 0x0000_FFFF_FFFF_FFFF
 * See https://www.disk91.com/2015/technology/sigfox/telecom-design-sdk-decode-gps-frame/
 *  for encoding detail
 * Basically
 * 444444443333333333222222222211111111110000000000
 * 765432109876543210987654321098765432109876543210
 * X                                                - lng Sign 1=-
 *  X                                               - lat Sign 1=-
 *   XXXXXXXXXXXXXXXXXXXXXXX                        - 23b Latitude
 *                          XXXXXXXXXXXXXXXXXXXXXXX - 23b Longitude
 *
 *  division by 215 for longitude is to get 180*10M to fit in 2^23b
 *  subtraction of 107 is 0.5 * 215 to round the value and not always be floored.
 */
static uint64_t gpsEncodePosition48b() 
{
  uint64_t t = 0;
  uint64_t l = 0;
  if ( gpsDate.longitude < 0 ) 
  {
    t |= 0x800000000000L;
    l = -gpsDate.longitude;
  } 
  else 
  {
    l = gpsDate.longitude;
  }
  if ( l/10000000 >= 180  ) 
  {
    l = 8372093;
  } 
  else 
  {
    if ( l < 107 ) 
    {
      l = 0;
    } 
    else 
    {
      l = (l - 107) / 215;
    }
  }
  t |= (l & 0x7FFFFF );

  if ( gpsDate.latitude < 0 ) 
  {
    t |= 0x400000000000L;
    l = -gpsDate.latitude;
  } 
  else 
  {
    l = gpsDate.latitude;
  }
  if ( l/10000000 >= 90  )
  {
    l = 8333333;
  } 
  else 
  {
    if ( l < 53 ) 
    {
      l = 0;
    } 
    else 
    {
      l = (l - 53) / 108;
    }
  }
  t |= (l << 23) & 0x3FFFFF800000;

  return t;
}

#if 1

time_t useTime;
void gpsRead()
{
  gpsDate.diplayState = false;
  gpsDate.txState = false;
  gpsDate.state = false;
	
	
	//useTime = millis();
	gpsDate.latitude    = GNSS.getLatitude();
	gpsDate.longitude   = GNSS.getLongitude();
	gpsDate.altitude    = GNSS.getAltitude();
	gpsDate.hdop 				= GNSS.getHorizontalDOP();
	gpsDate.sats 				= GNSS.getSIV();
	
	//useTime = millis() - useTime;
	//Serial.printf("GPS Use Time = %d \r\n",useTime);
	gpsStatus = FIXED_OK;
	if (( gpsDate.hdop < 200) && (gpsDate.sats  > 5))
	{
		GPS_LOG("GNSS , Lat: %.6f Lon: %.6f", gpsDate.latitude / 10000000.0, gpsDate.longitude / 10000000.0);
		GPS_LOG("GNSS , Alt: %.2f", gpsDate.altitude / 1000.0);
		GPS_LOG("GNSS , Acy: %.2f ", gpsDate.hdop / 100.0);
		GPS_LOG("GNSS , Sat: %d ", gpsDate.sats);
		if ((gpsDate.latitude == 0) && (gpsDate.longitude == 0))
		{
			gpsDate.diplayState = false;
			gpsDate.txState 		= false;
			gpsDate.state 			= false;
			gpsDate.hdop        = 0;
			gpsDate.latitude    = 0;
			gpsDate.longitude   = 0;
			gpsDate.altitude    = 0;
			gpsDate.sats        = 0;
			return;
		}
		else
		{
			gpsDate.encodePosition = gpsEncodePosition48b();
			gpsDate.frameDate[0]   = (gpsDate.encodePosition >> 40) & 0xFF;
			gpsDate.frameDate[1]   = (gpsDate.encodePosition >> 32) & 0xFF;
			gpsDate.frameDate[2]   = (gpsDate.encodePosition >> 24) & 0xFF;
			gpsDate.frameDate[3]   = (gpsDate.encodePosition >> 16) & 0xFF;
			gpsDate.frameDate[4]   = (gpsDate.encodePosition >> 8) & 0xFF;
			gpsDate.frameDate[5]   = (gpsDate.encodePosition) & 0xFF;
			gpsDate.frameDate[6]   = ((int)((float)gpsDate.altitude / 1000 + 1000) >> 8) & 0xFF;
			gpsDate.frameDate[7]   = ((int)((float)gpsDate.altitude / 1000 + 1000)) & 0xFF;
			gpsDate.frameDate[8]   = (uint8_t)gpsDate.hdop / 10;
			gpsDate.frameDate[9]   = gpsDate.sats;
			
			//g_solution_data.addGNSS_T(latitude, longitude, altitude, accuracy, satellites);
			gpsDate.diplayState = true;
			gpsDate.txState = true;
			gpsDate.state = true;
		}
	}
	else
	{
		gpsDate.hdop        = 0;
		gpsDate.latitude    = 0;
		gpsDate.longitude   = 0;
		gpsDate.altitude    = 0;
		gpsDate.sats        = 0;
	}

  return;
}

#else

void gpsRead(void)
{
	static uint32_t count = 0;
	
  memset(&gpsDate , '0' , sizeof(gpsDate_t));
	gpsDate.diplayState = true;
	gpsDate.txState = true;
	gpsDate.state = true;
	
	gpsDate.year    = 22;
	gpsDate.month   = 6;
	gpsDate.day     = 6;
	gpsDate.hour    = 9;
	gpsDate.minute  = 54;
	gpsDate.second  = 30;
	
	gpsStatus = FIXED_OK;
	
	//count +=10000;
  gpsDate.hdop        = 180;
  gpsDate.latitude    = 342109700 + count;
  gpsDate.longitude   = 1088399900 + count;
  gpsDate.altitude    = 486000;
  gpsDate.sats        = 10;
	gpsDate.speed				= 486;
  // GPS_LOG("GNSS  Lat: %.4f Lon: %.4f", gpsDate.latitude / 10000000.0, gpsDate.longitude / 10000000.0);
  // GPS_LOG("GNSS  Alt: %.2f", gpsDate.altitude / 1000.0);
  // GPS_LOG("GNSS  Acy: %.1f ", gpsDate.hdop / 10.0);
  // GPS_LOG("GNSS  Sat: %d ", gpsDate.sats);
  
  gpsDate.encodePosition = gpsEncodePosition48b();
  gpsDate.frameDate[0]   = (gpsDate.encodePosition >> 40) & 0xFF;
  gpsDate.frameDate[1]   = (gpsDate.encodePosition >> 32) & 0xFF;
  gpsDate.frameDate[2]   = (gpsDate.encodePosition >> 24) & 0xFF;
  gpsDate.frameDate[3]   = (gpsDate.encodePosition >> 16) & 0xFF;
  gpsDate.frameDate[4]   = (gpsDate.encodePosition >> 8) & 0xFF;
  gpsDate.frameDate[5]   = (gpsDate.encodePosition) & 0xFF;
  gpsDate.frameDate[6]   = ((gpsDate.altitude / 1000 + 1000) >> 8) & 0xFF;
  gpsDate.frameDate[7]   = ((gpsDate.altitude / 1000 + 1000)) & 0xFF;
  gpsDate.frameDate[8]   = (uint8_t)gpsDate.hdop / 10;
  gpsDate.frameDate[9]   = gpsDate.sats;
  
  //g_solution_data.addGNSS_T(latitude, longitude, altitude, accuracy, satellites);
}

#endif

int gpsEstimateDistance() 
{
  int64_t dLon = g_backupLongitude - gpsDate.longitude;
  int64_t dLat = g_backupLatitude  - gpsDate.latitude;
	
	LORA_LOG("Back GNSS  Lat: %ld", g_backupLatitude);
  LORA_LOG("Back GNSS  Lon: %ld", g_backupLongitude);
  
	LORA_LOG("GNSS  Lat: %ld", gpsDate.latitude);
  LORA_LOG("GNSS  Lon: %ld", gpsDate.longitude);
  
	LORA_LOG("dLat = %ld", dLat);
  LORA_LOG("dLon = %ld", dLon);
	
  if (dLon < 0) 
		dLon = -dLon;
  if (dLat < 0) 
		dLat = -dLat;
  dLon += dLat; 
  // We can estimate that a value of 100 = 1m
  if ( dLon > 1000000 ) 
		dLon = 1000000; // no need to be over 10km / preserve UI display in debug
  return (dLon/100); 
}

void gpsDeInit()
{
  
}

uint8_t getGpsStatus()
{
	return gpsStatus;	
}

void setGpsStatus()
{
	gpsStatus = NONE_FIXED;	
}

/*
  Helium console function for LoRaWan Field Tester sending mapping information to mappers backend.

  https://www.disk91.com/2021/technology/lora/low-cost-lorawan-field-tester/

  Result is visible on https://mappers.helium.com/

  Built from information available on:
    https://docs.helium.com/use-the-network/coverage-mapping/mappers-api/
    https://github.com/disk91/WioLoRaWANFieldTester/blob/master/WioLoRaWanFieldTester.ino
    https://www.disk91.com/2015/technology/sigfox/telecom-design-sdk-decode-gps-frame/

  Integration:
    POST https://mappers.helium.com/api/v1/ingest/uplink
*/
/*
function Decode(fPort, bytes) 
{ 
  var myObj = {"latitude":"", "longitude":"","altitude":"","accuracy":"","hdop":"","sats":""};
  var decoded = {};
  if (fPort === 1) 
  {
    var lonSign = (bytes[0]>>7) & 0x01 ? -1 : 1;
    var latSign = (bytes[0]>>6) & 0x01 ? -1 : 1;
    var encLat = ((bytes[0] & 0x3f)<<17)+ (bytes[1]<<9)+ (bytes[2]<<1)+ (bytes[3]>>7);
    var encLon = ((bytes[3] & 0x7f)<<16)+ (bytes[4]<<8)+ bytes[5];
    var hdop = bytes[8]/10;
    var sats = bytes[9];
    var maxHdop = 2;
    var minSats = 5;
    
    if ((hdop < maxHdop) && (sats >= minSats))
    {
      decoded.latitude  = latSign * (encLat * 108 + 53) / 10000000;
      decoded.longitude = lonSign * (encLon * 215 + 107) / 10000000;  
      decoded.altitude  = ((bytes[6]<<8)+bytes[7])-1000;
      decoded.accuracy  = (hdop*5+5)/10
      decoded.hdop      = hdop;
      decoded.sats      = sats;

      myObj.latitude  = decoded.latitude;
      myObj.longitude = decoded.longitude;
      myObj.altitude  = decoded.altitude;
      myObj.accuracy  = decoded.accuracy;
      myObj.hdop      = decoded.hdop;
      myObj.sats      = decoded.sats;
    } 
    else 
    {
      decoded.error = "Need more GPS precision (hdop must be <"+maxHdop+ " & sats must be >= "+minSats+") current hdop: "+hdop+" & sats:"+sats;
    }
    return myObj;
  }
  return null;
}
*/

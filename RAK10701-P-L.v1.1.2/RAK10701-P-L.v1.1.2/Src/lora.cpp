#include "../Inc/lora.h"
#include "../Inc/custom_at.h"
#include "../Inc/tft.h"
#include "../Inc/gps.h"

uint8_t g_sendIntervalLimit = 0;

uint8_t g_discoveryMode   = 0;

time_t  g_sendTime = 0;

discoveryStatus_t discoveryStatus  = 
{
  .sendCount 	= 0,    // Send 0 Date.
	.status     = 0,    // NONE.
	.update     = false,
};


/** Initialization results */
bool ret;

/** Lora Network Status Indication */
static uint8_t loraStatus = LORA_IDLE;

/** Lora Send Status Indication */
static uint8_t loraSendStatus = SEND_NONE;

/** Lora Rx Date Refresh Indication */
static uint8_t loraRxDateStatus = NOT_REFRESHED;

/** LoRa upload debug */
volatile uint16_t sendFailedCount  = 0;
volatile uint16_t sendSuccessCount = 0;
volatile uint64_t sendCount = 0;

/** OTAA Device EUI MSB */
uint8_t node_device_eui[8] = {0}; // ac1f09fff8683172
/** OTAA Application EUI MSB */
uint8_t node_app_eui[8] = {0}; // ac1f09fff8683172
/** OTAA Application Key MSB */
uint8_t node_app_key[16] = {0}; // efadff29c77b4829acf71e1a6e76f713

/** OTAA Application Key MSB */
hotspotsDate_t hotspotsDate = 
{
  .updateState 	= false,
	.origdate 		= {0},
	.mapDate 			= {0},
	.count 				= 0,
};
rssiDate_t rssiDate = 
{
  .updateState 	= false,
	.maxOrigdate 	= {0},
	.maxMapDate 	= {0},
	.minOrigdate 	= {0},
	.minMapDate 	= {0},
	.count 				= 0,
};

distanceDate_t distanceDate = 
{
  .updateState 	= false,
	.maxOrigdate 	= {0},
	.maxMapDate 	= {0},
	.minOrigdate 	= {0},
	.minMapDate 	= {0},
	.count 				= 0,
};
snrDate_t snrDate = 
{
  .updateState 	= false,
	.origdate 		= {0},
	.mapDate 			= {0},
	.count 				= 0,
};


void recvCallback(SERVICE_LORA_RECEIVE_T * data)
{
	LORA_LOG("+EVT:RX, port %d, DR %d, RSSI %d, SNR %d", data->Port, data->RxDatarate, data->Rssi, data->Snr);
	for (int i = 0; i < data->BufferSize; i++)
	{
		// LORA_LOG("%02X", data->Buffer[i]);
	}
	if (data->Port == 2)
	{
		int16_t min_rssi = data->Buffer[1] - 200;
		int16_t max_rssi = data->Buffer[2] - 200;
		int16_t min_distance = data->Buffer[3] * 250;
		int16_t max_distance = data->Buffer[4] * 250;
		uint16_t num_gateways = data->Buffer[5];
		
		addHotspotsDate(&hotspotsDate , num_gateways);
		addRssiDate(&rssiDate,max_rssi,min_rssi);
		addDistanceDate(&distanceDate,max_distance,min_distance);
		addSnrDate(&snrDate,data->Snr);
		
		loraRxDateStatus = REFRESHED;
		
		LORA_LOG("+EVT:FieldTester %d gateways", num_gateways);
		LORA_LOG("+EVT:RSSI min %d max %d", min_rssi, max_rssi);
		LORA_LOG("+EVT:Distance min %d max %d", min_distance, max_distance);
	}
}

void joinCallback(int32_t status)
{
  LORA_LOG("Join status: %d\r\n", status);

	if (status != 0)
	{
//		if (!(ret = api.lorawan.join()))
//		{
//			loraStatus = LORA_JOIN_FAILED;
//		}
		loraSendStatus = SEND_NONE;
		loraStatus = LORA_JOIN_FAILED;
		LORA_LOG("LoRaWan OTAA - join fail!");
	}
	else
	{
		loraSendStatus = SEND_NONE;
		loraStatus = LORA_JOINED;
		LORA_LOG("LoRaWan OTAA - join success!");
	}
/*
	if(api.lorawan.njs.get() == true)
	{
		loraStatus = LORA_JOINED;
	}
	else
	{
		loraStatus = LORA_JOIN_FAILED;
	}
	*/
}

void sendCallback(int32_t status)
{
  if (status == 0) 
	{
		sendSuccessCount++;
		loraSendStatus = SEND_OK;
    //LORA_LOG("Successfully send");
  } 
	else 
	{
		sendFailedCount++;
		loraSendStatus = SEND_FAILED;
    //LORA_LOG("Sending failed");
  }
	LORA_LOG("LoRaMacEventInfoStatus_t:%d Sending failed : %ld, Send Success: %ld",status , sendFailedCount , sendSuccessCount);
	loraStatus = LORA_IDLE;
}

initState_t loraInit()
{

	loraSendStatus = SEND_NONE;
	
	// if(bandChangeFlag == 0)
	// {
		// api.lorawan.band.set(g_curr_band);
	// }
	
	api.lorawan.nwm.set();
	
	if (api.lorawan.appeui.get(node_app_eui, 8))
	{
		// LORA_LOG("Got AppEUI %02X%02X%02X%02X%02X%02X%02X%02X",
				// node_app_eui[0], node_app_eui[1], node_app_eui[2], node_app_eui[3],
				// node_app_eui[4], node_app_eui[5], node_app_eui[6], node_app_eui[7]);
	}
	if (api.lorawan.appkey.get(node_app_key, 16))
	{
		// LORA_LOG("Got AppKEY %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
				// node_app_key[0], node_app_key[1], node_app_key[2], node_app_key[3],
				// node_app_key[4], node_app_key[5], node_app_key[6], node_app_key[7],
				// node_app_key[8], node_app_key[9], node_app_key[10], node_app_key[11],
				// node_app_key[12], node_app_key[13], node_app_key[14], node_app_key[15]);
	}
	if(g_netWorkMode == 1)
	{
		if (!api.lorawan.appkey.set(g_apps_KEY, 16)) 
		{
			api.lorawan.appkey.set(g_apps_KEY, 16);
		}
	}
	if (api.lorawan.deui.get(node_device_eui, 8))
	{
		// LORA_LOG("Got DevEUI %02X%02X%02X%02X%02X%02X%02X%02X",
				// node_device_eui[0], node_device_eui[1], node_device_eui[2], node_device_eui[3],
				// node_device_eui[4], node_device_eui[5], node_device_eui[6], node_device_eui[7]);
	}
	
	// Set the network join mode.  ABP:   0  OTAA:  1.
	api.lorawan.njm.set(1);
	
	// Set packet mode (confirmed/unconfirmed).
	// api.lorawan.cfm.set(0);
	
	// Turn off LoRa duty cycle limit.
	api.lorawan.dcs.set(0);
	
	// Start the join process.
	if (api.lorawan.join() == false)
	{
		LORA_LOG("LoRaWan OTAA - join fail! \r\n");
		//return INIT_LORA_FAILED;
	}
	
	api.lorawan.registerRecvCallback(recvCallback);
  api.lorawan.registerJoinCallback(joinCallback);
  api.lorawan.registerSendCallback(sendCallback);
	
	loraStatus = LORA_JOINING;
	
	return NONE;
}

void loraSend(gpsDate_t* frame)
{
	static time_t noMovementTimer = 0;
	static time_t backUpTimer			= 0;
	static uint8_t backupFlag = 0;
	
	if (api.lorawan.njs.get() == true) // Netwok  join.
  {
		loraStatus = LORA_SEND;
    LORA_LOG("Sending frame...");

    if(g_discoveryMode == 1)
    {
			g_sendTime = millis();
			if (api.lorawan.send(6, frame->frameDate, 3, false, 1))
			{
				LORA_LOG("Packet enqueued");
			}
//			else
//			{
//				LORA_LOG("Send failed");
//				loraSendStatus = SEND_FAILED;
//			}
      return;
    }
		
		//if(g_sendIntervalLimit == 0)
		if(backupFlag == 0)
		{
			backupFlag = 1;
			backUpTimer = millis();
			g_backupLongitude = gpsDate.longitude;
			g_backupLatitude  = gpsDate.latitude;
		}
	
		g_sendTime = millis();
    if (api.lorawan.send(sizeof(frame->frameDate), frame->frameDate, SEND_PORT, CONFIRM_MESSAGE, RETRY_TIMES))
    {
      LORA_LOG("Packet enqueued");
    }
    else
    {
      LORA_LOG("Send failed");
			loraSendStatus = SEND_FAILED;
    }
		
		/*
		 * In Helium mode, determine whether to limit the upload rate through the GPS position change.
		 */
		LORA_LOG("gpsEstimateDistance = %d ",gpsEstimateDistance());
		LORA_LOG("g_lorawanCfg.txInterval = %d ",g_lorawanCfg.txInterval);
		LORA_LOG("MAXNONMOVEMENT_DURATION_MS = %d ",MAXNONMOVEMENT_DURATION_MS);
		LORA_LOG("g_netWorkMode = %d ",g_netWorkMode);
		
		if((gpsEstimateDistance() < 100) && (g_lorawanCfg.txInterval <= MAXNONMOVEMENT_DURATION_MS) && (g_netWorkMode == 1))
		{
			LORA_LOG("noMovement...");
			//LORA_LOG("gpsEstimateDistance = %d ",gpsEstimateDistance());
			if(noMovementTimer == 0)
			{
				noMovementTimer = millis();
			}
			if( (millis() - noMovementTimer) > MAXNONMOVEMENT_DURATION_MS) // 900000
			{
				g_sendIntervalLimit = 1;
			}
		}
		else
		{
			noMovementTimer = 0;
			g_sendIntervalLimit = 0;
			if( (millis() - backUpTimer) > MAXNONMOVEMENT_DURATION_MS)
			{
				backupFlag = 0;
			}
		}
  }
  else  // Network not join. rejoin.
  {
		loraStatus = LORA_JOINING;
    LORA_LOG("Waiting for Lorawan join...");
		// Start the join process
		loraSendStatus = SEND_NONE;
		api.lorawan.join();
	//	if (api.lorawan.join() == false)
	//	{
	//		LORA_LOG("LoRaWan OTAA - join fail! \r\n");
	//		//return INIT_LORA_FAILED;
	//	}
  }
}

uint8_t getLoraSatus()
{
  return loraStatus;
}

uint8_t getLoraSendSatus()
{
  return loraSendStatus;
}

uint8_t getLoraRxDateStatus()
{
  return loraRxDateStatus;
}
void setLoraRxDateStatus()
{
	loraRxDateStatus = NOT_REFRESHED;
}

/*
 *@brief Shift hotspotsDate_t element right.
 */
void addHotspotsDate(hotspotsDate_t *date , uint16_t orDate)
{
	date->updateState = true;
	for(int i = RECORD_LENGTH - 1 ; i >0 ; i--)
	{
		date->origdate[i] = date->origdate[i-1];
		date->mapDate[i]  = date->mapDate[i-1];
	}
	
	/*
	@brief: Avoid over the graph.
	*/
	if(orDate > 30)	orDate = 30;		// avoid over the graph.	
	
	date->origdate[0] = orDate;
	date->mapDate[0]  = map(orDate , 0 , 30 , 148 , 70);
	
	if(date->count != RECORD_LENGTH)
	{
		date->count++;
	}
}

/*
 *@brief Shift rssiDate_t element right.
 */
void addRssiDate(rssiDate_t *date , int16_t orMaxDate, int16_t orMinDate)
{
	date->updateState = true;
	for(int i = RECORD_LENGTH - 1 ; i >0 ; i--)
	{
		date->maxOrigdate[i] = date->maxOrigdate[i-1];
		date->maxMapDate[i]  = date->maxMapDate[i-1];
		date->minOrigdate[i] = date->minOrigdate[i-1];
		date->minMapDate[i]  = date->minMapDate[i-1];
	}
	
	/*
	@brief: Avoid over the graph.
	*/
	if(orMaxDate > 20) 		orMaxDate = 20;		// avoid over the graph.
	if(orMaxDate < -130) 	orMaxDate = -130;	// avoid over the graph.	
	if(orMinDate > 20) 		orMinDate = 20;		// avoid over the graph.
	if(orMinDate < -130) 	orMinDate = -130;	// avoid over the graph.	
	
	date->maxOrigdate[0] = orMaxDate;
	date->maxMapDate[0]  = map(orMaxDate , 20 , -130 , 70 , 148);
	
	date->minOrigdate[0] = orMinDate;
	date->minMapDate[0]  = map(orMinDate , 20 , -130 , 70 , 148);
	
	if(date->count != RECORD_LENGTH)
	{
		date->count++;
	}
}

/*
 *@brief Shift distanceDate_t element right.
 */
void addDistanceDate(distanceDate_t *date , int16_t orMaxDate, int16_t orMinDate)
{
	date->updateState = true;
	for(int i = RECORD_LENGTH - 1 ; i >0 ; i--)
	{
		date->maxOrigdate[i] = date->maxOrigdate[i-1];
		date->maxMapDate[i]  = date->maxMapDate[i-1];
		date->minOrigdate[i] = date->minOrigdate[i-1];
		date->minMapDate[i]  = date->minMapDate[i-1];
	}
	/*
	@brief: Avoid over the graph.
	*/
	if(orMaxDate > 32000)	orMaxDate = 32000;
	if(orMaxDate < 0) 		orMaxDate = 0;				
	if(orMinDate > 32000)	orMinDate = 32000;		
	if(orMinDate < 0) 		orMinDate = 0;				
	
	date->maxOrigdate[0] = orMaxDate;
	date->maxMapDate[0]  = map(orMaxDate , 0 , 32000 , 148 , 70);
	
	date->minOrigdate[0] = orMinDate;
	date->minMapDate[0]  = map(orMinDate , 0 , 32000 , 148 , 70);
	
	if(date->count != RECORD_LENGTH)
	{
		date->count++;
	}
}

/*
 *@brief Shift snrDate_t element right.
 */
void addSnrDate(snrDate_t *date , int16_t orDate)
{
	date->updateState = true;
	for(int i = RECORD_LENGTH - 1 ; i >0 ; i--)
	{
		date->origdate[i] = date->origdate[i-1];
		date->mapDate[i]  = date->mapDate[i-1];
	}
	
	/*
	@brief: Avoid over the graph.
	*/
	if(orDate > 30)		orDate = 30;
	if(orDate < -30)	orDate = -30;	
	
	date->origdate[0] = orDate;
	date->mapDate[0]  = map(orDate , -30 , 30 , 148 , 70);
	
	if(date->count != RECORD_LENGTH)
	{
		date->count++;
	}
}


#include "../src/libraries/_Adafruit_GFX.h"	   // Click here to get the library: http://librarymanager/All#Adafruit_GFX
#include "../src/libraries/_Adafruit_ST7789.h" // Click here to get the library: http://librarymanager/All#Adafruit_ST7789
#include "../src/libraries/_FT6336U.h"
#include <SPI.h>

#include "../Inc/custom_at.h"

#include <Wire.h>								  //Needed for I2C to GNSS
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS

#include "radio.h"

#define CS SS
#define BL WB_IO3
#define RST WB_IO5
#define DC WB_IO4

#define RST_PIN WB_IO5
#define INT_PIN WB_IO6

Adafruit_ST7789 testTft = Adafruit_ST7789(CS, DC, RST);
FT6336U testFt6336u(RST_PIN, INT_PIN);
SFE_UBLOX_GNSS testGNSS;

const char *SEARCH_NAME = "RAK10701_TEST";
static char ble_scan_status = 0;

static uint8_t intSattus = 0;

static void keyIntHandle(void)
{
	intSattus = 1;
}

void TFT_TP_Test()
{
	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, HIGH);

	pinMode(BL, OUTPUT);
	digitalWrite(BL, HIGH); // Enable the backlight, you can also adjust the backlight brightness through PWM.

	testTft.init(240, 320); // Init ST7789 240x240.
	testTft.setRotation(3);

	testTft.fillScreen(ST77XX_RED);
	delay(2000);
	testTft.fillScreen(ST77XX_GREEN);
	delay(2000);
	testTft.fillScreen(ST77XX_BLUE);
	delay(2000);

	testFt6336u.begin(); // No reset. Reset is reset when the screen is reset
	pinMode(INT_PIN, INPUT_PULLUP);

	pinMode(RST_PIN, INPUT_PULLUP);
	// attachInterrupt(digitalPinToInterrupt(INT_PIN), keyIntHandle, FALLING);

	// testFt6336u.disable_face_dec_mode();
	// testFt6336u.write_time_period_enter_monitor(0);

	intSattus = 0;

	testTft.setTextSize(1);
	testTft.setCursor(20, 50);
	testTft.setTextColor(ST77XX_WHITE);
	testTft.setTextWrap(true);
	testTft.print("Touch the screen and then push the button within 8s.");
	Serial.println("Touch the screen and then push the button within 8s.");

	testTft.setCursor(20, 100);
	time_t time = millis();
	uint8_t pressCount = 0;
	while ((millis() - time) < 5000)
	{
		if ((digitalRead(INT_PIN) == 0) && (pressCount == 0))
		{
			pressCount = 1;
		}
		if ((digitalRead(RST_PIN) == 0) && (pressCount == 1))
		{
			pressCount = 2;
		}
		if (pressCount == 2)
		{
			testTft.print("TP Test OK!");
			Serial.printf("TP Test OK!\n");
			digitalWrite(BL, LOW); // Enable the backlight, you can also adjust the backlight brightness through PWM.
			return;
		}
	}
	testTft.setCursor(20, 100);
	testTft.print("TP Test Failed!");
	Serial.printf("TP Test Failed!\n");
	digitalWrite(BL, LOW);
}

// Define LoRa parameters
#define RF_FREQUENCY 868300000	// Hz
#define TX_OUTPUT_POWER 22		// dBm
#define LORA_BANDWIDTH 0		// [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1		// [1: 4/5, 2: 4/6,  3: 4/7,  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8	// Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0	// Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 3000
#define TX_TIMEOUT_VALUE 3000

static char lora_rx_count = 0;
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	// if(memcmp(payload,"RAK",strlen("RAK")) == 0)
	// {
	// lora_rx_count++;
	// Serial.printf("Count:%d RSSI:%d \r\n",lora_rx_count,rssi);
	// }
	Serial.printf("Count:%d RSSI:%d \r\n", lora_rx_count, rssi);
}

void OnRxTimeout(void)
{
}

bool rx_done = false;
double myFreq = 868300000;
uint16_t sf = 7, bw = 125, cr = 0, preamble = 8, txPower = 22;

void recv_cb(rui_lora_p2p_recv_t data)
{
	rx_done = true;
	if (data.BufferSize == 0)
	{
		Serial.println("Empty buffer.");
		return;
	}
	char buff[92];
	sprintf(buff, "Incoming message, length: %d, RSSI: %d, SNR: %d", data.BufferSize, data.Rssi, data.Snr);
	Serial.println(buff);
	lora_rx_count++;
}
void LORA_Test()
{
	unsigned long LocalMillis;
	Serial.println("P2P Start");
	Serial.printf("Set Node device work mode %s\r\n", api.lora.nwm.set() ? "Success" : "Fail");
	Serial.printf("Set P2P mode frequency %3.3f: %s\r\n", (myFreq / 1e6), api.lora.pfreq.set(myFreq) ? "Success" : "Fail");
	Serial.printf("Set P2P mode spreading factor %d: %s\r\n", sf, api.lora.psf.set(sf) ? "Success" : "Fail");
	Serial.printf("Set P2P mode bandwidth %d: %s\r\n", bw, api.lora.pbw.set(bw) ? "Success" : "Fail");
	Serial.printf("Set P2P mode code rate 4/%d: %s\r\n", (cr + 5), api.lora.pcr.set(0) ? "Success" : "Fail");
	Serial.printf("Set P2P mode preamble length %d: %s\r\n", preamble, api.lora.ppl.set(8) ? "Success" : "Fail");
	Serial.printf("Set P2P mode tx power %d: %s\r\n", txPower, api.lora.ptp.set(22) ? "Success" : "Fail");
	api.lora.registerPRecvCallback(recv_cb);
	Serial.printf("P2P set Rx mode %s\r\n", api.lora.precv(5000) ? "Success" : "Fail");
	LocalMillis = millis();

	lora_rx_count = 0;
	while (millis() - LocalMillis < 5000)
	{
		if (rx_done)
		{
			rx_done = false;
			Serial.printf("P2P set Rx mode %s\r\n", api.lora.precv(3000) ? "Success" : "Fail");
		}
		delay(500);
	}
	Serial.printf("Number of LoRa packets received:%d.\r\n", lora_rx_count);
}

void string2hexString(char *input, char *output, uint16_t data_len)
{
	int loop;
	int i;
	i = 0;
	loop = 0;

	while (loop < data_len)
	{
		sprintf((char *)(output + i), "%02X ", input[loop]);
		loop += 1;
		i += 3;
	}
	output[i++] = '\0';
}

static uint8_t ble_Test_Flag = 0;
void scan_callback(int8_t rssi_value, uint8_t *device_mac, uint8_t *scan_data, uint16_t data_len)
{
	char ptr_ofs = 0;
	if (ble_Test_Flag == 0)
		return;

	while (SEARCH_NAME[ptr_ofs])
	{
		if (scan_data[ptr_ofs + 8] != SEARCH_NAME[ptr_ofs])
		{
			return;
		}
		ptr_ofs++;
	}
	Serial.printf("MAC:");
	for (int i = 5; i >= 0; i--)
		Serial.printf("%02X ", device_mac[i]);
	Serial.printf(" RSSI:%d \r\n", rssi_value);
	ble_scan_status = 1;
	ble_Test_Flag = 0;
}
void BLE_Test()
{
	static uint8_t flag = 0;
	unsigned long LocalMillis;

	ble_Test_Flag = 1;
	ble_scan_status = 0;
	Serial.printf("Bluetooth scan max 15 seconds.\r\n");
	// udrv_ble_stop();
	// api.ble.scanner.start(0);
	if (flag == 0)
	{
		flag = 1;
		api.ble.scanner.start(0);
		if (true != api.ble.scanner.setInterval(1000, 500))
		{
			Serial.printf("Ble scanner start failed.\r\n");
			Serial.printf("Bluetooth test failure.\r\n");
			return;
		}
		api.ble.scanner.setScannerCallback(scan_callback);
	}
	// api.ble.scanner.start(0);

	LocalMillis = millis();
	while (millis() - LocalMillis < 15000)
	{
		delay(1000);
		if (ble_scan_status != 0)
		{
			Serial.printf("Bluetooth test passed.\r\n");
			// api.ble.scanner.stop();
			return;
		}
		Serial.print(".");
	}
	// api.ble.scanner.stop();
	Serial.printf("No Bluetooth device was scanned.\r\n");
	Serial.printf("Bluetooth test failure.\r\n");
}
void GPS_Test()
{
	Wire.begin();

	long latitude = 0;
	long longitude = 0;
	long altitude = 0;

	if (testGNSS.begin() == false) // Connect to the u-blox module using Wire port
	{
		Serial.printf("GPS Test Failed!\n");
		return;
	}
	testGNSS.setI2COutput(COM_TYPE_UBX);				 // Set the I2C port to output UBX only (turn off NMEA noise)
	testGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); // Save (only) the communications port settings to flash and BBR

	unsigned long time_now = millis();
	while ((millis() - time_now) < 120000)
	{
		latitude = testGNSS.getLatitude();
		longitude = testGNSS.getLongitude();
		altitude = testGNSS.getAltitude();

		if ((latitude != 0) && (longitude != 0) && (altitude != 0) && (testGNSS.getGnssFixOk()))
		{
			Serial.println();
			Serial.print(F(" Lat: "));
			Serial.print(latitude);
			Serial.print(F(" Long: "));
			Serial.print(longitude);
			Serial.print(F(" (degrees * 10^-7)"));
			Serial.print(F(" Alt: "));
			Serial.print(altitude);
			Serial.print(F(" (mm)"));
			Serial.println();
			Serial.printf("GPS Test OK!\r\n");
			return;
		}
		latitude = 0;
		longitude = 0;
		altitude = 0;
		Serial.print(".");
		delay(1000);
	}
	Serial.printf("GPS Test Failed!\r\n");
}
void enterfactoryTestMode()
{
	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, HIGH);
	while (1)
	{
		if (get_test_mode() == 0)
		{
			Serial.printf("Start test TFT & TP.\r\n");
			TFT_TP_Test();
			Serial.printf("Test end.\r\n");
			set_test_mode();
		}
		else if (get_test_mode() == 1)
		{
			Serial.printf("Start test LORA.\r\n");
			LORA_Test();
			Serial.printf("Test end.\r\n");
			set_test_mode();
		}
		else if (get_test_mode() == 2)
		{
			Serial.printf("Start test BLE.\r\n");
			BLE_Test();
			Serial.printf("Test end.\r\n");
			set_test_mode();
		}
		else if (get_test_mode() == 3)
		{
			Serial.printf("Start test GPS.\r\n");
			GPS_Test();
			Serial.printf("Test end.\r\n");
			set_test_mode();
		}
		delay(100);
	}
}

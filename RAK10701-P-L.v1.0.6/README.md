## Getting started

### **Step 1. Add the RAKwireless RUI nRF Boards**

1. Open your Arduino IDE, click on **File** > **Preferences**, and copy below url to **Additional Boards Manager URLs**:

   https://raw.githubusercontent.com/RAKWireless/RAKwireless-Arduino-BSP-Index/main/package_rakwireless.com_rui_index.json

2. Click on **Tools** > **Board** > **Board Manager** and Search **RAKwireless RUI nRF Boards** in the Boards Manager.

3. Install version v3.5.1.

### **Step 2. Copy library files**

1. Copy the required library files to your Arduino library directory, the directory may look like this:C:\Users\RAK\Documents\Arduino\libraries

### **Step 3. Select your board and port**

1. You'll need to select the entry in the **Tools > Board** menu that corresponds to your Arduino. Selecting the **WisBlock Core RAK4631 Board**.

   Select the serial device of the RAK4631 Board from the **Tools -> Port** menu. This is likely to be COM3 or higher (**COM1** and **COM2** are usually reserved for hardware serial ports). To find out, you can disconnect your **Feild Tester** and re-open the menu; the entry that disappears should be the Arduino board. Reconnect the board and select that serial port.

   **Note:**

   For Mac User, it will be something like `/dev/cu.usbmodem141401`

### **Step 4. Upload the program**

1. Now, simply click the **Upload** button in the environment. Wait a few seconds and if the upload is successful, the message **Done uploading.** will appear in the status bar.

## Frame format

1. The following Frame format are used:
   **uplink format on port 1:**

| Byte    | Usage                                                        |
| ------- | ------------------------------------------------------------ |
| `0 - 5` | GSP position see [here](https://www.disk91.com/2015/technology/sigfox/telecom-design-sdk-decode-gps-frame/) for details. Decoding see below |
| `6 - 7` | Altitude in meters + 1000m ( 1100 = 100m )                   |
| `8`     | HDOP * 10 (11 = 1.1)                                         |
| `9`     | Sats in view                                                 |

2. When the GPS position is invalid of GPS is disable, the frame is full of 0

   **downlink response format on port 2:**

| Byte | Usage                         |
| ---- | ----------------------------- |
| `0`  | Sequence ID % 255             |
| `1`  | Min Rssi + 200 (160 = -40dBm) |
| `2`  | Max Rssi + 200 (160 = -40dBm) |
| `3`  | Min Distance step 250m        |
| `4`  | Max Distance step 250m        |
| `5`  | Seen hotspot                  |

​	The distance is calculated from the GPS position and the hotspot position returned by console meta-data. Under 250m value is 250m, over 32km value is 32km. 0 is considered as invalid response

​	The following integration and payload transformation allows to decode the gps position and report is to mapper. Thank you Seb for the contribution.

​	**Dicovery uplink format on port 3 (no ack):**

| Byte    | Usage                                                        |
| ------- | ------------------------------------------------------------ |
| `0 - 5` | GSP position see [here](https://www.disk91.com/2015/technology/sigfox/telecom-design-sdk-decode-gps-frame/) for details. Decoding see below |

​	Discovery is sending 10 messages SF10 on every 40 seconds. All the other information comes from the metadata provided by the network server.


## Decoder for the frame format

​	Such a decoder can be use if you do not want to use my backend to report the coordinate to the helium mapper

​	Create a _Functions_ type _Decoder_ / _Custom Script_ and attach it to a mapper integration callback as it is described in this [helium mapper integration page](https://docs.helium.com/use-the-network/coverage-mapping/mappers-quickstart/)

```js
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

function Decoder(bytes, port) { 
  var decoded = {};
  // avoid sending Downlink ACK to integration (Cargo)
  if (port === 1) {
    var lonSign = (bytes[0]>>7) & 0x01 ? -1 : 1;
    var latSign = (bytes[0]>>6) & 0x01 ? -1 : 1;
    
    var encLat = ((bytes[0] & 0x3f)<<17)+
                 (bytes[1]<<9)+
                 (bytes[2]<<1)+
                 (bytes[3]>>7);
  
    var encLon = ((bytes[3] & 0x7f)<<16)+
                 (bytes[4]<<8)+
                 bytes[5];
    
    var hdop = bytes[8]/10;
    var sats = bytes[9];
    
    const maxHdop = 2;
    const minSats = 5;
    
    if ((hdop < maxHdop) && (sats >= minSats)) {
      // Send only acceptable quality of position to mappers
      decoded.latitude = latSign * (encLat * 108 + 53) / 10000000;
      decoded.longitude = lonSign * (encLon * 215 + 107) / 10000000;  
      decoded.altitude = ((bytes[6]<<8)+bytes[7])-1000;
      decoded.accuracy = (hdop*5+5)/10
      decoded.hdop = hdop;
      decoded.sats = sats;
    } else {
      decoded.error = "Need more GPS precision (hdop must be <"+maxHdop+
        " & sats must be >= "+minSats+") current hdop: "+hdop+" & sats:"+sats;
    }
    return decoded;
  }
    return null;
}
```

## RAK10701 Instruction manual

​	Please refer to [documentation](https://docs.rakwireless.com/Product-Categories/WisNode/RAK10701-P/Overview).
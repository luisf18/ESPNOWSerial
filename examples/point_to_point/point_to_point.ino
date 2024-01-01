/*/ 
    Point to Point connection
    Closed connection between two devices
    Device1 <---> Device2
/*/

#include "ESPNOWSerial.h"

// Comment to compile for device 2
#define device_1


#ifdef device_1
  // Insert here MAC of device 2
  uint8_t send_MAC[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#else
  // Insert here MAC of device 1
  uint8_t send_MAC[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#endif

void setup() {

  Serial.begin(115200);
  Serial.setTimeout(20);

  // ESPNOWSerial begin
  ESPNOWSerial.begin( send_MAC ); // when you add a mac it only sends to that mac. (point to point)
  
  // ajust timing
  ESPNOWSerial.setTimeout(20);
  ESPNOWSerial.setWriteDelay(10);
  
  // filter to only receive data from the "send_mac"
  ESPNOWSerial.canReciveFrom_SendMacOnly();

  // print Hello World
  #ifdef device_1
  ESPNOWSerial.printf( "ESPNOWSerial, Hello World from device 1!\n" );
  #else
  ESPNOWSerial.printf( "ESPNOWSerial, Hello World from device 2!\n" );
  #endif

}

void loop() {

  // Print in the serial if there are characters available
  if( ESPNOWSerial.available() > 0){
    String msg = ESPNOWSerial.readString();
    Serial.println( msg );
  }

}

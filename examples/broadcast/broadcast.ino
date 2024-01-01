/*/ 
    Broadcast connection
    One device sends to all other devices

               /-> Device2
    Device1 <---> Device3
               \-> Device4 ...
/*/

#include "ESPNOWSerial.h"

void setup() {

  Serial.begin(115200);
  Serial.setTimeout(20);

  // ESPNOWSerial begin
  ESPNOWSerial.begin(); // without parameters it goes into broadcast mode
  
  // ajust timing
  ESPNOWSerial.setTimeout(20);
  ESPNOWSerial.setWriteDelay(10);
  
  // no filter, can recive from any other device
  ESPNOWSerial.canReciveFrom_anyDevice();

  // print Hello World
  ESPNOWSerial.printf( "ESPNOWSerial broadcasting, Hello World!\n" );

}

void loop() {

  // Print in the serial if there are characters available
  if( ESPNOWSerial.available() > 0){
    String msg = ESPNOWSerial.readString();
    Serial.println( msg );
  }
  
}

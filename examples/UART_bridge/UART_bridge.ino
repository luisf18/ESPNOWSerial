#include "ESPNOWSerial.h"

void setup() {
  
  Serial.begin(115200);
  Serial.setTimeout(20);

  // ESPNOWSerial
  ESPNOWSerial.begin();
  ESPNOWSerial.setTimeout(20);
  ESPNOWSerial.setWriteDelay(10);
  ESPNOWSerial.canReciveFrom_anyDevice();

  ESPNOWSerial.printf( "ESPNOWSerial, Hello World!\n" );

}

void loop() {
  
  if( Serial.available() > 0){
    String msg = Serial.readStringUntil('\n');
    ESPNOWSerial.println( msg );
  }

  if( ESPNOWSerial.available() > 0){
    String msg = ESPNOWSerial.readString();
    Serial.println( msg );
  }

}

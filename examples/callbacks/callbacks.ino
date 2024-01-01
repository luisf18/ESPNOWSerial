/*/ 
    Callbacks

    It is possible to add callback functions
    that are called after sending or receiving

/*/

#include "ESPNOWSerial.h"


// Callback functions
void onRecive(const uint8_t *mac, const uint8_t *data, int len){
  Serial.printf( "[callback recive] recive new data from %s\n", ESPNOWSerial.mac2str(mac).c_str() );
}

void onSent( const uint8_t *mac ){
  Serial.printf( "[callback sent] sent data to %s\n", ESPNOWSerial.mac2str(mac).c_str() );
}

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

  // Callbacks
  ESPNOWSerial.setReciveCallback(onRecive);
  ESPNOWSerial.setSendCallback(onSent);

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

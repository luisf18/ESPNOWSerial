#ifndef ESPNOW_SERIAL_H
#define ESPNOW_SERIAL_H

//#include "Arduino.h"
#include <Arduino.h>

#ifdef ESP32
  #include <esp_mac.h>
  #include <esp_now.h>
  #include <WiFi.h>
  #include <esp_wifi.h>
#elif defined(ESP8266)
  #include <espnow.h>
  #include <ESP8266WiFi.h>
#endif

// number of characters it can receive
#define ESPNOWSerial_BUFFER_len 1000

#include "Stream.h"

class espnowSerial: public Stream {

  private:

  #ifdef ESP32
  esp_now_peer_info_t peerInfo;
  #endif

  // debug
  boolean debug_flag = false;

  // 2.4GHz band
  uint8_t Channel = 1;

  // send
  uint8_t  send_address[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  uint8_t  send_status = 0; // 0: repouso / 1: falhou no envio / 2: enviado com sucesso
  uint32_t send_timeout = 0;
  uint32_t write_dt = 5;
  void wait_send();

  // Read BUFFER
  uint8_t  BUFFER[ESPNOWSerial_BUFFER_len];
  uint32_t BUFFER_i    = 0;
  uint32_t BUFFER_size = 0;
  void shift_left_buffer(uint32_t n);
  void clear_read_buffer();

  // Auxiliary callback functions
  void (*cb_recive)(const uint8_t*,const uint8_t*,int) = nullptr;
  void (*cb_send)(const uint8_t*) = nullptr;

  // last recive time
  uint32_t last_recive_ms = 0;

  // Recive modes
  uint8_t recive_mode = ANY_MAC;
  uint8_t recive_mac_list[10][8];
  uint8_t recive_mac_list_len = 0;

public:

  //enum{ COMBO = 0, RX, TX };

  // Recive mode
  enum{
    ANY_MAC = 0,
    ONLY_FROM_SEND_MAC,
    ONLY_FROM_MAC_LIST
  };

  espnowSerial(){}

  uint8_t broadcast[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  // Begin
  bool begin();
  bool begin(uint8_t *send_mac);
  bool begin(uint8_t *send_mac, uint8_t channel, uint8_t *self_mac );
  bool begin(uint8_t *send_mac, uint8_t channel );

  // deinit
  void deinit(){
    esp_now_deinit();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
  }

  // peer, set_mac
  bool peer( uint8_t *mac );
  void set_mac( uint8_t *new_mac );

  // Recive modes
  void canReciveFrom_anyDevice();
  void canReciveFrom_SendMacOnly();
  int  canReciveFrom_MacList( uint8_t *list, uint8_t n );
  int  find_mac( const uint8_t *mac, const uint8_t *list, uint8_t list_len );
  
  // Auxiliary callback functions
  void setReciveCallback( void (*f)(const uint8_t*,const uint8_t*,int) ){ cb_recive = f; }
  void setSendCallback( void (*f)(const uint8_t*) ){ cb_send = f; }
  
  // Minimal callback functions
  void class_onRecive(const uint8_t * mac,const uint8_t *incomingData, int len);
  void class_onSent(const uint8_t *mac, esp_now_send_status_t status);
  
  /////////////////////// OVERRIDE //////////////////////////
  
  int available(){ return (BUFFER_size - BUFFER_i); };
  
  // Read
  int peek();
  int read();
  size_t readBytes(char *buffer, size_t length);
  //virtual size_t readBytes(uint8_t *buffer, size_t length){ return readBytes((char *) buffer, length); }
  //virtual String readString();

  // write
  void     setWriteDelay(uint32_t t){ write_dt = t; }
  uint32_t getWriteDelay(){ return write_dt; }
  size_t   write(uint8_t a);
  size_t   write(const uint8_t *buffer, size_t size);

  // misc
  String   mac2str(const uint8_t *mac );
  uint32_t getLastReciveTime();
  boolean  debug(){ return debug_flag; }
  void     debug( boolean flag ){ debug_flag = flag; }

};


// *********************************************************************** //
// Class ESPNOWSerial                                                      //
// *********************************************************************** //

extern espnowSerial ESPNOWSerial;


#endif
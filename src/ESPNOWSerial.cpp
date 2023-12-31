#include "ESPNOWSerial.h"

espnowSerial ESPNOWSerial;

// *********************************************************************** //
// Declaration of external callback functions                              //
// *********************************************************************** //
#ifdef ESP32
void onRecive(const uint8_t * mac,const uint8_t *incomingData, int len);
void onSent(const uint8_t *mac_addr, esp_now_send_status_t status);
#elif defined(ESP8266)
void onRecive(uint8_t * mac, uint8_t *incomingData, uint8_t len);
void onSent(uint8_t *mac_addr, uint8_t sendStatus);
#endif


// *********************************************************************** //
// Private                                                                 //
// Buffer functions and wait_send                                          //
// *********************************************************************** //

void espnowSerial::shift_left_buffer(uint32_t n){
    if( n > ESPNOWSerial_BUFFER_len ) n = ESPNOWSerial_BUFFER_len;
    if( n > BUFFER_size ) n = BUFFER_size;
    BUFFER_size -= n;
    BUFFER_i    -= n;
    for(int i=0;i<BUFFER_size;i++)
        *(BUFFER+i) = *(BUFFER+n+i);
}

void espnowSerial::clear_read_buffer(){
    shift_left_buffer(BUFFER_i);
}

void espnowSerial::wait_send(){
    send_timeout = millis() + 100;
    while( send_timeout > millis() ){
        if( send_status > 0 ) break;
    }
    send_status = 0;
}


// *********************************************************************** //
// Public                                                                  //
// Begin functions                                                         //
// *********************************************************************** //
bool espnowSerial::begin(){ return begin(send_address,1); }
bool espnowSerial::begin(uint8_t *send_mac){ return begin(send_mac,1); }
bool espnowSerial::begin(uint8_t *send_mac, uint8_t channel, uint8_t *self_mac ){
    set_mac( self_mac );
    return begin( send_mac, channel );
}
bool espnowSerial::begin(uint8_t *send_mac, uint8_t channel ){
    
    // Init ESPNOW --------------------------------  
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    // ESP_ERROR_CHECK( esp_wifi_set_channel(ESPNOW_CH,WIFI_SECOND_CHAN_NONE) );
    if(esp_now_init() != 0){
        Serial.println("[Error] initializing ESP-NOW");
        return false;
    }
    Serial.println("\n\nBEGIN ESPNOW!!");
    // ---------------------------------------------

    //ESP_ERROR_CHECK( esp_wifi_set_channel(ESPNOW_CH,WIFI_SECOND_CHAN_NONE) );

    #ifndef ESP32
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    #endif

    // callback recive
    esp_now_register_recv_cb(onRecive);
    esp_now_register_send_cb(onSent);


    // Register peer device
    if(!peer( send_mac )) return false;


    uint8_t primary;
    wifi_second_chan_t second;
    esp_wifi_get_channel(&primary, &second);
    Serial.printf("[ESPNOW] channel: %d   %d\n",primary,second);

    Serial.print("Board self MAC Address:  ");
    Serial.println(WiFi.macAddress());
    return true;
}


// *********************************************************************** //
// Public                                                                  //
// Peer and set_mac                                                        //
// *********************************************************************** //

bool espnowSerial::peer( uint8_t *mac ){
    memcpy(send_address,mac,6);
    // Register peer device
    #ifdef ESP32
    memcpy(peerInfo.peer_addr, send_address, 6);
    peerInfo.channel = 0; //ESPNOW_CH;
    peerInfo.encrypt = false;
    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK){ Serial.println("Failed to add peer"); return false; }
    #else
    esp_now_add_peer(broadcast, ESP_NOW_ROLE_SLAVE, 0 /*ESPNOW_CH*/, NULL, 0);
    #endif
    return true;
}

void espnowSerial::set_mac( uint8_t *new_mac ){
    #if ESP32
        //esp_wifi_set_max_tx_power(84);
        //ESP32 Board add-on before version < 1.0.5
        //esp_wifi_set_mac(ESP_IF_WIFI_STA, &newMACAddress[0]);
        // ESP32 Board add-on after version > 1.0.5
        esp_wifi_set_mac(WIFI_IF_STA, new_mac);
    #else
        // For Soft Access Point (AP) Mode
        //wifi_set_macaddr(SOFTAP_IF, &newMACAddress[0]);
        // For Station Mode
        wifi_set_macaddr(STATION_IF, new_mac);
    #endif
}



// *********************************************************************** //
// Public                                                                  //
// Recive modes: any, SendMac or MacList                                   //
// find_mac                                                                //
// *********************************************************************** //

void espnowSerial::canReciveFrom_anyDevice(){ recive_mode = ANY_MAC; }
void espnowSerial::canReciveFrom_SendMacOnly(){ recive_mode = ONLY_FROM_SEND_MAC; }
int  espnowSerial::canReciveFrom_MacList( uint8_t *list, uint8_t n ){
    if(n==0) return -1;
    recive_mode = ONLY_FROM_MAC_LIST;
    recive_mac_list_len = constrain(n,0,10);
    memcpy( recive_mac_list, list, recive_mac_list_len*6 );
    return recive_mac_list_len;
}
int espnowSerial::find_mac( const uint8_t *mac, const uint8_t *list, uint8_t list_len ){
    for(int i=0;i<list_len;i++){
        if( memcmp( list, mac, 6 ) == 0 ) return i;
        list+=6;
    }
    return -1;
}


// *********************************************************************** //
// Public                                                                  //
// minimal callback functions: class_onRecive, class_onSent                //
// *********************************************************************** //

void espnowSerial::class_onRecive(const uint8_t * mac,const uint8_t *incomingData, int len){
    
    uint32_t t = millis();
    
    // debug
    if(debug_flag){
        Serial.printf( "RECIVE [%i]", len ); String STR_MAC = mac2str(mac); Serial.printf( "[%s] ", STR_MAC.c_str());
        //Serial.write( incomingData, len );
    }

    // Verifica o codigo MAC
    if( recive_mode == ONLY_FROM_MAC_LIST ){
        if( find_mac( mac, &recive_mac_list[0][0], recive_mac_list_len ) == -1 ) return;
    }else if( recive_mode == ONLY_FROM_SEND_MAC ){
        if( memcmp(mac,send_address,6) != 0 ) return;
    }

    // ---- validação concluida! ---- //
    last_recive_ms = t;

    // função de callback auxiliar
    if( cb_recive != nullptr ) cb_recive( mac, incomingData, len );

    // descarta dados ja lidos
    clear_read_buffer();

    // armazena os dados no BUFFER
    int available_len = ESPNOWSerial_BUFFER_len - BUFFER_size;
    if( available_len > 0 ){
        if(available_len < len) len = available_len;
        memcpy( BUFFER + BUFFER_size, (uint8_t*) incomingData, len);
        BUFFER_size+=len;
    }

    // debug
    if(debug_flag) Serial.printf( " - END RECIVE [ %i ms ]\n", millis() - t );

}

void espnowSerial::class_onSent(const uint8_t *mac, esp_now_send_status_t status){
    if( cb_send != nullptr ) cb_send( mac );
    boolean ok = (status == ESP_NOW_SEND_SUCCESS);
    send_status = 1 + ok;
}

// *********************************************************************** //
// Public - Basic functions                                                //
// Read functions: peek and read                                           //
// *********************************************************************** //

int espnowSerial::peek(){
    if( BUFFER_size == BUFFER_i ) return -1;
    return *(BUFFER+BUFFER_i);
}

int espnowSerial::read(){
    if( BUFFER_size == BUFFER_i ) return -1;
    int a = *(BUFFER+BUFFER_i);
    BUFFER_i++;
    return a;
}
  
size_t espnowSerial::readBytes(char *buffer, size_t length){
    if( BUFFER_size == BUFFER_i ) return -1;
    if( length > BUFFER_size ) length = BUFFER_size;
    memcpy( (uint8_t*) buffer, (uint8_t*) BUFFER, length);
    BUFFER_i += length;
    return length;
}

// *********************************************************************** //
// Public - Basic functions                                                //
// Write functions: write                                                  //
// *********************************************************************** //

size_t espnowSerial::write(uint8_t a){
    esp_now_send(send_address, &a, 1);
    return 1;
}

size_t espnowSerial::write(const uint8_t *buffer, size_t size){
    send_status = 0;
    size_t n = size;
    while( n > 0 ){
      uint32_t len = ( n > 250 ? 250 : n );
      esp_now_send(send_address, buffer, len);
      n -= len;
      buffer += len;
      wait_send();
      if(write_dt>0) delay(write_dt);
    }
    return size;
}


// *********************************************************************** //
// Public                                                                  //
// misc functions: mac2str                                                 //
// *********************************************************************** //

String espnowSerial::mac2str(const uint8_t *mac ){
    char char_str[18];
    snprintf(char_str, sizeof(char_str), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    String ret = char_str;
    return ret;
}


// *********************************************************************** //
// External callback functions                                             //
// *********************************************************************** //
#ifdef ESP32
void onRecive(const uint8_t * mac,const uint8_t *incomingData, int len){
    ESPNOWSerial.class_onRecive( mac, incomingData, len);
}
void onSent(const uint8_t *mac, esp_now_send_status_t status) {
    ESPNOWSerial.class_onSent( mac, status);
}
#elif defined(ESP8266)
// ....
#endif

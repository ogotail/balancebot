// =============================================================================
// ===                            WIFI                                       ===
// =============================================================================

#include "WifiCom.h"

//************   Creation   ************
Wifi::Wifi(){
    IPAddress ipMulti( 192,168,0,255 );
    // A UDP instance to let us send and receive packets over UDP
}

//************   Connection   ************
void Wifi::connect(){
    // essaye de ce connecter a un reseau
    // en premier a celui donne
    WiFi.mode( WIFI_STA );
    WiFi.begin( ssid, password );
    // si la connection est un echec
    if( WiFi.waitForConnectResult() != WL_CONNECTED ){
        // cree un reseau
        //Serial.println( "Connection Failed! Configuring access point..." );
        //Serial.print( "ESSID : testapesp   IP : " );
        WiFi.softAP( "testapesp" );
        IPAddress myIP = WiFi.softAPIP();
        //Serial.println( myIP );
    }
    Udp.begin( localPort );
}

//************   Envoie   ************
void Wifi::send(String msg){
    // envoie le message au client
    Udp.beginPacket( Ip, Port ); //Send message to Packet-Sender
    Udp.write( msg.c_str() );
    Udp.endPacket();
}

//************   Reception   ************
String Wifi::read(){
    int packetSize = Udp.parsePacket();
    if ( packetSize ){
        CONNECTED = 1;
        Ip = Udp.remoteIP() ;
        Port = Udp.remotePort();

        // We've received a packet, read the data from it
        Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
        // read the packet into the buffer
        return packetBuffer ;
    }
    return "";
}

// =============================================================================
// ===                            OTA                                        ===
// =============================================================================

//************   Initialisation   ************
void Wifi::InitOta(){
    // Configure la mise a jour par wifi
    // Action a faire avant la mise a jour
    ArduinoOTA.onStart([](){});
    // Action a faire apres la mise a jour ( avant reboot )
    ArduinoOTA.onEnd([](){});
    // Action a faire pendans la mise a jour
    ArduinoOTA.onProgress([]( unsigned int progress, unsigned int total ){});
    // Action a faire en cas d erreur de la mise a jour
    ArduinoOTA.onError([]( ota_error_t error ){});
    ArduinoOTA.begin();
    //Serial.println( "OTA Mode Ready" );
}

//************   Routine   ************
void Wifi::checkOta(){
    // a mettre dans la boucle pour attendre une mise a jour
    ArduinoOTA.handle();
}

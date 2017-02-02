// =============================================================================
// ===                            WIFI                                       ===
// =============================================================================

#include "WifiCom.h"

// ************   WIFI   ************
const char* ssid = "CathyMath";
const char* password = "4362626262";

//************   Variables Udp  ************
// local port to listen for UDP packets
unsigned int localPort = 2390;
IPAddress ipMulti ( 192,168,0,255 );
// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
IPAddress Ip ;
unsigned int Port ;
//buffer to hold incoming and outgoing packets
char packetBuffer[ UDP_TX_PACKET_MAX_SIZE ];
bool CONNECTED = 0 ;

//************   Creation   ************
Wifi::Wifi(){/*nothing to Creat*/}

//************   Destruction   ************
Wifi::~Wifi(){/*nothing to destruct*/}

void Wifi::connect(){
    // essaye de ce connecter a un reseau
    // en premier a celui donne
    //Serial.print( "Connection at : " );
    //Serial.println( ssid );
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
    //else{
    //    Serial.print( "Connected ! IP : " );
    //    Serial.println( WiFi.localIP() );
    //}
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

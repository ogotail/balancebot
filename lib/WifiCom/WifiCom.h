#ifndef WIFICOM_H
#define WIFICOM_H

#include <Arduino.h>

// ************   WIFI   ************
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

// ************   UDP   ************
#include <WiFiUdp.h>

// ************   OTA   ************
#include <ArduinoOTA.h>

class Wifi{
    private:
        // local port to listen for UDP packets
        unsigned int localPort = 2390;
        unsigned int Port ;
        //buffer to hold incoming and outgoing packets
        char packetBuffer[ UDP_TX_PACKET_MAX_SIZE ];
        bool CONNECTED = 0 ;
        const char* ssid = "WIFI_NAME";
        const char* password = "123456789";
        WiFiUDP Udp;
        IPAddress Ip ;

    public:
        Wifi();
        void connect();
        void send( String msg );
        String read();
        void InitOta();
        void checkOta();
};

#endif

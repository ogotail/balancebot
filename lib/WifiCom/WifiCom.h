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
    public:
        Wifi();
        ~Wifi();
        void connect();
        void send( String msg );
        String read();
        void InitOta();
        void checkOta();
};

#endif

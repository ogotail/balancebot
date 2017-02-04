// =============================================================================
// ===                         Librairie                                     ===
// =============================================================================

#include <Arduino.h>

// ************   FILE   ************
#include <FS.h>

// ************   WIFI   ************
#include <WifiCom.h>

// ************   MPU   ************
#include <MPU.h>

// ************   ENCODER   ************
// need https://github.com/PaulStoffregen/Encoder
#include <Encoder.h>

//************   MOTEURS   ************
// choix du driver des moteurs
//#include "Motor6612.h"
#include "Motor8833.h"

//************   BATTERY   ************
#include "Battery.h"

//************   PID   ************
//#include "PID.h"

// =============================================================================
// ===                      PIN Configuration                                ===
// =============================================================================

//************   ENCODER   ************
// ENCODER Left
#define ENCODER_L_F  4
#define ENCODER_L_B  5
// ENCODER Right
#define ENCODER_R_F  1
#define ENCODER_R_B  15

// =============================================================================
// ===                       Variable Global                                 ===
// =============================================================================

String MODE = "" ;

// ************   COMMUNICATION   ************
String SEND = "" ;

//************   ENCODER   ************
Encoder EncL( ENCODER_L_F, ENCODER_L_B );
Encoder EncR( ENCODER_R_F, ENCODER_R_B );

//************   MPU   ************
MPU mpu;

//************   WIFI   ************
Wifi wifi;

//************   MOTOR   ************
Motor Mot;

//************   PID   ************
//PID Pid;

// =============================================================================
// ===                          ENCODER                                      ===
// =============================================================================

//************   Initialisation   ************

void EncLF(){wifi.send( "encoder L F " + String( digitalRead( ENCODER_L_F )));}

void EncLB(){wifi.send( "encoder L B " + String( digitalRead( ENCODER_L_B )));}

void EncRF(){wifi.send( "encoder R F " + String( digitalRead( ENCODER_R_F )));}

void EncRB(){wifi.send( "encoder R B " + String( digitalRead( ENCODER_R_B )));}

void InitManEnc(){
    pinMode( ENCODER_L_F, INPUT_PULLUP );
    pinMode( ENCODER_L_B, INPUT_PULLUP );
    pinMode( ENCODER_R_F, INPUT_PULLUP );
    pinMode( ENCODER_R_B, INPUT_PULLUP );
    attachInterrupt( digitalPinToInterrupt( ENCODER_L_F ), EncLF, CHANGE );
    attachInterrupt( digitalPinToInterrupt( ENCODER_L_B ), EncLB, CHANGE );
    attachInterrupt( digitalPinToInterrupt( ENCODER_R_F ), EncRF, CHANGE );
    attachInterrupt( digitalPinToInterrupt( ENCODER_R_B ), EncRB, CHANGE );
}

void StopEnc(){
    detachInterrupt( digitalPinToInterrupt( ENCODER_L_F ) );
    detachInterrupt( digitalPinToInterrupt( ENCODER_L_B ) );
    detachInterrupt( digitalPinToInterrupt( ENCODER_R_F ) );
    detachInterrupt( digitalPinToInterrupt( ENCODER_R_B ) );
}

// =============================================================================
// ===                          BATTERY                                      ===
// =============================================================================
Battery bat;

//************   cherche la valeur de la batterie   ************
void get_Battery() { wifi.send( "bat " + String(bat.get()) ); }

// =============================================================================
// ===                            FILE                                       ===
// =============================================================================

//************   LOAD   ************
bool loadConfig(){
    SPIFFS.begin() ;
    // open file for reading.
    File configFile = SPIFFS.open( "/balancebot/conf.txt", "r" );
    if (!configFile) return false;
    // Read content from config file.
    String content = configFile.readString();
    configFile.close();

    String commande = content.substring( 0, content.indexOf( " " ) );

    if ( commande == "PID" ){
        content = content.substring( content.indexOf( " " ) + 1 );
       while ( !content.startsWith( "\r\n" ) ){
            int pos = content.indexOf( " ", content.indexOf( " " ) + 1 );
            //Pid.set( content.substring( 0, pos ));
            content = content.substring( pos + 1 );
        }
    }
    return true;
}

//************   SAVE   ************
bool saveConfig(){
    // Open config file for writing.
    File configFile = SPIFFS.open( "/balancebot/conf.txt", "w+" );
    if ( !configFile ) return false;
    // Save PID
    //configFile.println( Pid.get() );
    configFile.close();
    return true;
}

// =============================================================================
// ===                          PARSER                                       ===
// =============================================================================

void receivemsg(){
    String msg = wifi.read();
    if ( msg && msg != "" ){
        msg.trim();
        wifi.send( "ar " + msg );
        String commande = msg.substring( 0, msg.indexOf(' ') );
        String data = msg.substring( msg.indexOf(' ') + 1 );

        if ( commande == "set" ){
            //Pid.set( data ) ;
        }
        else if ( commande == "MODE" ){
            MODE = data.substring(0, data.indexOf(' '));
        }
        else if ( commande == "get" ){
            SEND = data.substring(0, data.indexOf(' '));
        }
        else if ( commande == "PID" ){
            //wifi.send( Pid.get() ) ;
        }
        else if ( commande == "save" ){
            wifi.send("save " + String( saveConfig()));
        }
        else if ( commande == "PAUSE" ){
            if ( data.startsWith( "1" ) ) Mot.set_Pause( true );
            else Mot.set_Pause( false );
            wifi.send( "PAUSE " + String( Mot.get_Pause() ));
        }
        else {
            wifi.send( "? " + msg );
        }
    }
}

// =============================================================================
// ===                           PID                                        ===
// =============================================================================

float pitch ;
float gyro ;
float Kp = 20 ;
float Ki = 10 ;
float Kd = 0.01 ;
float Kr = 10 ;

void PID(){
    long encL = EncL.read() ;
    long encR = EncR.read() ;
    mpu.update( &pitch, &gyro );
    if ( pitch > 30 || pitch < -30 ) {
        Mot.Stop();
        if ( SEND == "Stability" ){
             wifi.send( "Stability " + String( millis() ) + " "
                        + String( pitch ) + " 0 0 0 0 "
            );
        }
    }
    else {
        int speed = pitch * Kp + ( encL + encR ) * Ki + gyro * Kd ;
        int rot = ( encL - encR ) * Kr ;
        Mot.Speed( speed, rot );
        if ( SEND == "Stability" ){
            wifi.send( "Stability " + String( millis() ) + " "
                        + String( pitch ) + " "
                        + String( speed ) + " "
                        + String( pitch * Kp ) + " "
                        + String( ( encL + encR ) * Ki ) + " "
                        + String( gyro * Kd ) + " "
            );
        }
    }
}

// =============================================================================
// ===                           MODE                                        ===
// =============================================================================

//************   Run   ************
void ModeRun(){
    mpu.init_sensor();
    loadConfig();
    while ( MODE == "RUN" ){
        receivemsg() ;
        get_Battery();
        PID();
    }
}

//************   OTA   ************
void ModeOTA(){
    wifi.InitOta();
    while ( MODE == "OTA" ){
        receivemsg() ;
        get_Battery();
        wifi.checkOta();
    }
}

//************   Cal   ************
void  Modecal(){
    mpu.init_sensor();
    InitManEnc();
    while ( MODE == "CAL" ){
        receivemsg() ;
        get_Battery();
    }
    StopEnc();
}

// =============================================================================
// ===                           MAIN                                        ===
// =============================================================================

void setup(){
    wifi.connect();
    get_Battery();
}

void loop(){
    receivemsg();
    get_Battery();
    if ( MODE == "OTA" ) ModeOTA() ;
    else if ( MODE == "RUN" ) ModeRun();
    else if ( MODE == "CAL" ) Modecal();
}

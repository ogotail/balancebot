// =============================================================================
// ===                         Librairy                                      ===
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

// =============================================================================
// ===                            DEBUG                                      ===
// =============================================================================

#define TEST_MOTOR
#define DEBUG_MOTOR

#define TEST_MPU
#define DEBUG_MPU

#define DEBUG_WIFI

#define DEBUG_PID

// =============================================================================
// ===                      PIN Configuration                                ===
// =============================================================================

//************   ENCODER   ************
// ENCODER Left
#define ENCODER_L_F  D1
#define ENCODER_L_B  D2
// ENCODER Right
#define ENCODER_R_F  D7
#define ENCODER_R_B  D6

//************   MOTEURS   ************
// Motor Left
#define MOTOR_L_F  10
#define MOTOR_L_B  9
// MOTOR Right
#define MOTOR_R_F  D5
#define MOTOR_R_B  D8

//************   MPU   ************
#define MPU_SDA  D3
#define MPU_SCL  D4

// =============================================================================
// ===                       Variable Global                                 ===
// =============================================================================

String MODE = "RUN" ;

// ************   COMMUNICATION   ************
String SEND = "Stability" ;

//************   ENCODER   ************
Encoder EncL( ENCODER_L_F, ENCODER_L_B );
Encoder EncR( ENCODER_R_F, ENCODER_R_B );

//************   MPU   ************
MPU mpu( MPU_SDA, MPU_SCL );

//************   WIFI   ************
Wifi wifi;

//************   MOTOR   ************
Motor Mot( MOTOR_L_F, MOTOR_L_B, MOTOR_R_F, MOTOR_R_B );

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
void get_Battery() { wifi.send( "bat " + String( bat.get() )); }

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

int PID = 0 ;
float pitch ;
float P = 0 ;
int I = 0 ;
float gyro ;
float D = 0 ;
int cor_g = 240 ;

int ROT = 0 ;

float Kp = 100 ;
float Ki = 0 ;
float Kd = 0.1 ;
float Kr = 0 ;

void send_PID(){
    string msg = (
                "Stability "
                + String( millis() ) + " "
                + String( pitch ) + " "
                + String( PID ) + " "
                + String( P ) + " "
                + String( I ) + " "
                + String( D ) + " "
    );
    if ( SEND == "Stability" ){ wifi.send( msg );}
    #ifdef DEBUG_PID
        Serial.println( msg );
    #endif
}

void PID(){
    mpu.update( &pitch, &gyro );
    if ( pitch > 30 || pitch < -30 ) {
        Mot.Stop();
        if ( SEND == "Stability" ){ s_stab( pitch, 0, 0, 0, 0 );}
    }
    else {
        P = ( P + kp * pitch ) / 2 ;
        long encL = EncL.read() ;
        long encR = EncR.read() ;
        I = ( encL + encR )* Ki ;
        D = ( D - (gyro + cor_g) * Kd ) / 2 ;
        PID = P + I + D ;
        ROT = ( encL - encR ) * Kr ;
        Mot.Speed( PID, ROT );
        send_PID()
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

#ifdef NORMAL
    void setup(){
        if ( !Serial ) Serial.begin( 115200 );
        Serial.println( F( "INIT" ));
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
#endif

// =============================================================================
// ===                           TEST                                        ===
// =============================================================================

#ifdef TEST_MPU
    #define DEBUG_MPU
    void setup() {
        Serial.begin( 115200 );
        mpu.init_sensor();
    }
    void loop() {
        delay( 1000 );
        mpu.update();
    }
#endif

#ifdef TEST_MOTOR
    #define DEBUG_MOTOR
    void setup() { Serial.begin( 115200 ); }
    void loop() { Mot.test() ;}
#endif

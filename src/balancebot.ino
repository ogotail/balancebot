// =============================================================================
// ===                         Librairie                                     ===
// =============================================================================

#include <Arduino.h>

// ************   FILE   ************
#include <FS.h>

// ************   WIFI   ************
#include <WifiCom.h>

// ************   IMU   ************
// need https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/MPU6050
#include "MPU6050_6Axis_MotionApps20.h"
//#include "IMU.h"

// ************   ENCODER   ************
// need https://github.com/PaulStoffregen/Encoder
#include <Encoder.h>

//************   MOTEURS   ************
// choix du driver des moteurs
#include "Motor6612.h"
//#include "Motor8833.h"

//************   BATTERY   ************
#include "Battery.h"

//************   PID   ************
#include "PID.h"

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

//************   IMU   ************
//IMU Imu6050;

//************   WIFI   ************
Wifi wifi;

//************   MOTOR   ************
Motor Mot;

//************   PID   ************
PID Pid;

// =============================================================================
// ===                            IMU                                        ===
// =============================================================================

//************   IMU   ************
// I2C
#define IMU_SDA 10
#define IMU_SCL 9
// interuption
#define IMU_INT 14

//************   Variables   ************
MPU6050 mpu;

// status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
//uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU

// MPU control
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

//************   Initialisation   ************
void InitImu(){
    // join I2C bus (I2Cdev library doesn't do this automatically)
    Wire.begin( IMU_SDA, IMU_SCL );
    // initialize device
    mpu.initialize();
    // verify connection
    mpu.testConnection();
    // load and configure the DMP
    devStatus = mpu.dmpInitialize();
    // Setup Accel offsets
    mpu.setXAccelOffset(-7029);
    mpu.setYAccelOffset(-1294);
    mpu.setZAccelOffset(1200);
    // Setup gyro offsets
    mpu.setXGyroOffset(147);
    mpu.setYGyroOffset(72);
    mpu.setZGyroOffset(25);
    // make sure it worked (returns 0 if so)
    if (devStatus == 0){
        // turn on the DMP, now that it's ready
        mpu.setDMPEnabled(true);
        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        dmpReady = true;
        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    }
}

//************   Mise a jour des angles   ************
float UpdateAngles(){
    if ( dmpReady ){
        // get current FIFO count
        fifoCount = mpu.getFIFOCount();
        // verifie si un packet est present
        // A CHANGER !!! pour utiliser les interuptions ( if => while )
        if ( fifoCount < packetSize) return 0;
        else {
            // read all packet from FIFO until the last
            while (fifoCount >= packetSize){
                mpu.getFIFOBytes(fifoBuffer, packetSize);
                fifoCount = mpu.getFIFOCount();
            }
            // nettoie les miettes
            if (fifoCount != 0) mpu.resetFIFO();
            // Transformation geometrique
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
            // et renvoie l angle par rapport la vertical en degree
            return (ypr[1] * 180/M_PI);
        }
    }
    else return 0;
}

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
            Pid.set( content.substring( 0, pos ));
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
    configFile.println( Pid.get() );
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
            Pid.set( data ) ;
        }
        else if ( commande == "MODE" ){
            MODE = data.substring(0, data.indexOf(' '));
        }
        else if ( commande == "get" ){
            SEND = data.substring(0, data.indexOf(' '));
        }
        else if ( commande == "PID" ){
            wifi.send( Pid.get() ) ;
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
// ===                           MODE                                        ===
// =============================================================================

//************   Run   ************
void ModeRun(){
    InitImu();
    loadConfig();
    while ( MODE == "RUN" ){
        receivemsg() ;
        get_Battery();
        float angle = UpdateAngles();
        if ( !angle || angle > 30 || angle < -30 ) {
            Mot.Stop();
            if ( SEND == "Stability" ){
                 wifi.send( "Stability " + String( millis() ) + " "
                 + String( angle ) + " 0 0 0 0 ");
            }
        }
        else {
            Mot.Speed( Pid.stab( angle ) );
            if ( SEND == "Stability" ){ wifi.send( Pid.get_sta()); }
        }
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
    InitImu();
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

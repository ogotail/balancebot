// ================================================================
// ===                          MOTORS                          ===
// ================================================================

#include "Motor6612.h"

//************   MOTEURS   ************

// Motor Left
#define MOTOR_L_F  16
#define MOTOR_L_B  0
#define MOTOR_L_S  2
// MOTOR Right
#define MOTOR_R_F  12
#define MOTOR_R_B  13
#define MOTOR_R_S  3

bool PAUSE = true;

//************   Initialisation   ************

Motor6612::Motor6612(){
    //Motor Left SETUP
    pinMode(MOTOR_L_S, OUTPUT);
    analogWrite(MOTOR_L_S, 0);
    pinMode(MOTOR_L_F, OUTPUT);
    digitalWrite(MOTOR_L_F, LOW);
    pinMode(MOTOR_L_B, OUTPUT);
    digitalWrite(MOTOR_L_B, LOW);

    //Motor Right SETUP
    pinMode(MOTOR_R_S, OUTPUT);
    analogWrite(MOTOR_R_S, 0);
    pinMode(MOTOR_R_F, OUTPUT);
    digitalWrite(MOTOR_R_F, LOW);
    pinMode(MOTOR_R_B, OUTPUT);
    digitalWrite(MOTOR_R_B, LOW);

    analogWriteRange(255);
}

//************   Destruction   ************

Motor6612::~Motor6612(){
    /*nothing to destruct*/
}

//************   change la vitesse des moteurs   ************

void Motor6612::Write(int speedL, int speedR){
    // ajuste la vitesse et direction des MOTORS

    ////Serial.println( "MOTOR_L : " + String( speedL ) + "MOTOR_R : " + String( speedR ) );
    if( speedR >= 0 ){
        digitalWrite( MOTOR_L_B, LOW);
        analogWrite( MOTOR_L_S, speedR );
        digitalWrite( MOTOR_L_F, HIGH);
        //dirD = 1;
    }
    else{
        digitalWrite( MOTOR_L_F, LOW);
        analogWrite( MOTOR_L_S, -speedR );
        digitalWrite( MOTOR_L_B, HIGH);
        //dirD = -1;
    }
    if( speedL >= 0 ){
        digitalWrite( MOTOR_R_B, LOW);
        analogWrite( MOTOR_R_S, speedL );
        digitalWrite( MOTOR_R_F, HIGH);
        //dirG = 1;
    }
    else{
        digitalWrite( MOTOR_R_F, LOW);
        analogWrite( MOTOR_R_S, -speedL );
        digitalWrite( MOTOR_R_B, HIGH);
        //dirG = -1;
    }
}

//************   arret des moteurs   ************

void Motor6612::Stop(){
    // stop les MOTORs => roues libre
    analogWrite( MOTOR_L_S, 0 );
    analogWrite( MOTOR_R_S, 0 );
}

//************   change la vitesse des moteurs   ************

void Motor6612::Speed( int speedM, int diff ){
    if ( !PAUSE ) Write( speedM + diff, speedM - diff );
    else Stop();
}

//************   freine les moteurs   ************

void Motor6612::Brake(){
    // Freine les MOTORs => roues Bloquees
    digitalWrite( MOTOR_L_F, HIGH );
    digitalWrite( MOTOR_L_B, HIGH );
    digitalWrite( MOTOR_R_F, HIGH );
    digitalWrite( MOTOR_R_B, HIGH );
    analogWrite( MOTOR_L_S, 255 );
    analogWrite( MOTOR_R_S, 255 );

    delay(100); // relache les roues apres 100ms pour pas tout cramer
    digitalWrite( MOTOR_L_F, LOW );
    digitalWrite( MOTOR_L_B, LOW );
    digitalWrite( MOTOR_R_F, LOW );
    digitalWrite( MOTOR_R_B, LOW );
    analogWrite( MOTOR_L_S, 0 );
    analogWrite( MOTOR_R_S, 0 );
}

// ================================================================
// ===                          MOTORS                          ===
// ================================================================

#include "Motor8833.h"

//************   MOTEURS   ************
// Motor Left
#define MOTOR_L_F  0
#define MOTOR_L_B  2
// MOTOR Right
#define MOTOR_R_F  12
#define MOTOR_R_B  13

//************   Creation   ************
Motor::Motor(){
    analogWriteRange(255);
    //Motor Left SETUP
    pinMode(MOTOR_L_F, OUTPUT);
    analogWrite(MOTOR_L_F, 0);
    pinMode(MOTOR_L_B, OUTPUT);
    analogWrite(MOTOR_L_B, 0);
    //Motor Right SETUP
    pinMode(MOTOR_R_F, OUTPUT);
    analogWrite(MOTOR_R_F, 0);
    pinMode(MOTOR_R_B, OUTPUT);
    analogWrite(MOTOR_R_B, 0);
}

//************   change la vitesse des moteurs   ************
void Motor::Write(int speedL, int speedR){
    // ajuste la vitesse et direction des MOTORS
    if( speedR >= 0 ){
        analogWrite( MOTOR_L_F, speedR +55 );
        analogWrite( MOTOR_L_B, 0 );
    }
    else{
        analogWrite( MOTOR_L_B, -speedR +55 );
        analogWrite( MOTOR_L_F, 0 );
    }
    if( speedL >= 0 ){
        analogWrite( MOTOR_R_F, speedL +55 );
        analogWrite( MOTOR_R_B, 0 );
    }
    else{
        analogWrite( MOTOR_R_B, -speedL +55 );
        analogWrite( MOTOR_R_F, 0 );
    }
}

//************   arret des moteurs   ************
void Motor::Stop(){
    // stop les MOTORs => roues libre
    analogWrite( MOTOR_L_F, 0 );
    analogWrite( MOTOR_L_B, 0 );
    analogWrite( MOTOR_R_F, 0 );
    analogWrite( MOTOR_R_B, 0 );
}

//************   change la vitesse des moteurs   ************
void Motor::Speed( int speedM, int diff ){
    if ( !PAUSE ) Write( (speedM + diff) , (speedM - diff) );
    else Stop();
}

//************   freine les moteurs   ************
void Motor::Brake(){
    // Freine les MOTORs => roues Bloquees
    analogWrite( MOTOR_L_F, 255 );
    analogWrite( MOTOR_L_B, 255 );
    analogWrite( MOTOR_R_F, 255 );
    analogWrite( MOTOR_R_B, 255 );

    delay(100); // relache les roues apres 100ms pour pas tout cramer
    analogWrite( MOTOR_L_F, 0 );
    analogWrite( MOTOR_L_B, 0 );
    analogWrite( MOTOR_R_F, 0 );
    analogWrite( MOTOR_R_B, 0 );
}

void Motor::set_Pause( bool state ){
    Stop();
    PAUSE = state ;
}

bool Motor::get_Pause(){ return PAUSE ;}

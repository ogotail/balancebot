// ================================================================
// ===                          MOTORS                          ===
// ================================================================

#include "Motor8833.h"

// Motor Left
int pmLF ;
int pmLB ;
// MOTOR Right
int pmRF ;
int pmRB ;

bool pause = 0 ;

// =============================================================================
// ===                            DEBUG                                      ===
// =============================================================================

//#define DEBUG_MOTOR

//************   Creation   ************
Motor::Motor( int pmlf, int pmlr, int pmrf, int pmrb ){
    // pin mapping
    pmLF = pmlf ;
    pmLB = pmlr ;
    pmRF = pmrf ;
    pmRB = pmrb ;

    //analogWriteRange(255);
    //Motor Left SETUP
    pinMode(pmLF, OUTPUT);
    analogWrite(pmLF, 0);
    pinMode(pmLB, OUTPUT);
    analogWrite(pmLB, 0);
    //Motor Right SETUP
    pinMode(pmRF, OUTPUT);
    analogWrite(pmRF, 0);
    pinMode(pmRB, OUTPUT);
    analogWrite(pmRB, 0);
}

//************   change la vitesse des moteurs   ************
void Motor::Write(int speedL, int speedR){
    // ajuste la vitesse et direction des MOTORS
    if( speedR >= 0 ){
        analogWrite( pmLF, speedR );
        analogWrite( pmLB, 0 );
    }
    else{
        analogWrite( pmLB, -speedR );
        analogWrite( pmLF, 0 );
    }
    if( speedL >= 0 ){
        analogWrite( pmRF, speedL );
        analogWrite( pmRB, 0 );
    }
    else{
        analogWrite( pmRB, -speedL );
        analogWrite( pmRF, 0 );
    }
}

//************   arret des moteurs   ************
void Motor::Stop(){
    // stop les MOTORs => roues libre
    analogWrite( pmLF, 0 );
    analogWrite( pmLB, 0 );
    analogWrite( pmRF, 0 );
    analogWrite( pmRB, 0 );
}

//************   change la vitesse des moteurs   ************
void Motor::Speed( int speedM, int diff ){
    if ( !pause ){
        Write(
            constrain( speedM + diff, -1024, 1024 ),
            constrain( speedM - diff, -1024, 1024 )
        );
    }
    else Stop();
}

//************   freine les moteurs   ************
void Motor::Brake(){
    // Freine les MOTORs => roues Bloquees
    analogWrite( pmLF, 254 );
    analogWrite( pmLB, 254 );
    analogWrite( pmRF, 254 );
    analogWrite( pmRB, 254 );

    delay(100); // relache les roues apres 100ms pour pas tout cramer
    analogWrite( pmLF, 0 );
    analogWrite( pmLB, 0 );
    analogWrite( pmRF, 0 );
    analogWrite( pmRB, 0 );
}

void Motor::set_Pause( bool state ){
    Stop();
    pause = state ;
}

bool Motor::get_Pause(){ return pause ;}

void Motor::test(){
    #ifdef DEBUG_MOTOR
        if ( !Serial ) Serial.begin( 115200 );
        set_Pause( 0 ) ;
        Serial.println( F( "Test Pins = pmLF" ));
        digitalWrite( pmLF, HIGH );
        delay( 1000 );
        digitalWrite( pmLF, LOW );
        Serial.println( F( "Test Pins = pmLB" ));
        digitalWrite(pmLB, HIGH);
        delay( 1000 );
        digitalWrite( pmLB, LOW );
        Serial.println( F( "Test Pins = pmRF" ));
        digitalWrite( pmRF, HIGH );
        delay( 1000 );
        digitalWrite( pmRF, LOW );
        Serial.println( F( "Test Pins = pmRB" ));
        digitalWrite( pmRB, HIGH );
        delay( 1000 );
        digitalWrite( pmRB, LOW );

        Serial.println( F( "Test motors" ));
        int speed = 0 ;
        while( speed < 1024 ){
            speed += 10 ;
            if ( speed ) Serial.println( speed );
            Speed( speed );
            delay( 100 );
        }
        delay( 1000 );
        while( speed > -1024 ){
            speed -= 10 ;
            if ( speed ) Serial.println( speed );
            Speed( speed );
            delay( 100 );
        }
        delay( 1000 );
        while( speed < 0 ){
            speed += 10 ;
            if ( speed ) Serial.println( speed );
            Speed( speed ) ;
            delay( 100 );
        }
        Stop();
    #endif
}


// ================================================================
// ===                          BATTERY                         ===
// ================================================================

#include "Battery.h"


// variables pour la battery
const int AvgC = 10;
float prevC[AvgC];
int prevCI = 0;
long batTemp;
int batNotRready = AvgC;

Battery::Battery(){
    pinMode( A0, INPUT ) ;
    prevC[prevCI] = analogRead( A0 ) / 1024.0 * 10 ;
    batTemp = millis() + 100 ;
}

Battery::~Battery(){
    /*nothing to destruct*/
}

float Battery::get(){
    if ( batNotRready ){
        prevCI = ( prevCI + 1 ) % AvgC ;
        prevC[ prevCI ] = analogRead( A0 ) / 1024.0 * 10 ;  // lecture de la batterie en volt
        float sum = 0 ;
        for ( int i = 0; i < prevCI; i++ ) sum += prevC[i] ;
        batNotRready -= 1 ;
        return sum / prevCI - 0.08;
    }
    else if ( batTemp < millis() ){
        prevCI = (prevCI + 1) % AvgC ;
        prevC[prevCI] = analogRead( A0 ) / 1024.0 * 10 ;  // lecture de la batterie en volt
        float sum = 0 ;
        for ( int i = 0; i < AvgC; i++ ) sum += prevC[i] ;
        batTemp = millis() + 500 ;
        return sum / AvgC - 0.08 ;
    }
}

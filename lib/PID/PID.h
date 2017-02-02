#ifndef PID_H
#define PID_H

#include <Arduino.h>

class PID{
    public:
        PID();
        ~PID();
        void set( String data );
        String get();
        float stab(float Angle, float correction = 0 );
        String get_sta();
        float dep( int EncL, int EncR, int consigne = 0 );
        String get_dep();
        float rot( int EncL, int EncR, int consigne = 0 );
        String get_rot();
};

#endif

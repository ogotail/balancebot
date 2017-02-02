#ifndef Motor6612_H
#define Motor6612_H

#include <Arduino.h>

class Motor{
    public:
        Motor();
        ~Motor();
        void Write( int speedL, int speedR );
        void Stop();
        void Speed( int speedM, int diff=0 );
        void Brake();
        void set_Pause( bool state );
        bool get_Pause();
};

#endif

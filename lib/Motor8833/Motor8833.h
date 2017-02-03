#ifndef Motor8833_H
#define Motor8833_H

#include <Arduino.h>

class Motor{
    private:
        bool PAUSE = true;
        void Write( int speedL, int speedR );
    public:
        Motor();
        void Stop();
        void Speed( int speedM, int diff=0 );
        void Brake();
        void set_Pause( bool state );
        bool get_Pause();
};

#endif

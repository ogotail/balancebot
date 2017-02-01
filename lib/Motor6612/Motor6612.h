#ifndef Motor6612_H
#define Motor6612_H

#include <Arduino.h>

class Motor6612{
    public:
        Motor6612();
        ~Motor6612();
        void Write(int speedL, int speedR);
        void Stop();
        void Speed( int speedM, int diff = 0 );
        void Brake();
};

#endif

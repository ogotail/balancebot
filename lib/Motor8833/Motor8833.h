#ifndef Motor8833_H
#define Motor8833_H

#include <Arduino.h>

class Motor{
    public:
        Motor();
        ~Motor();
        void Write(int speedL, int speedR);
        void Stop();
        void Speed(int speedM, int diff=0);
        void Brake();
};

#endif

#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

class Battery{
    public:
        Battery();
        ~Battery();
        char get();
};

#endif

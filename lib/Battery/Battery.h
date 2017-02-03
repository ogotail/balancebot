#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

class Battery{
    private:
        const int AvgC = 10;
        float prevC[10];
        int prevCI = 0;
        long batTemp;
        int batNotRready = AvgC;
        float bat;
    public:
        Battery();
        float get();
};

#endif

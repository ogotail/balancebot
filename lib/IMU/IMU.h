#ifndef IMU_H
#define IMU_H

#include <Arduino.h>

// ************   I2C   ************
#include <Wire.h>

// need i2cdevlib MPU6050
// https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/MPU6050
#include "MPU6050_6Axis_MotionApps20.h"

class IMU{
    public:
        IMU();
        ~IMU();
        void Initialisation();
        float Angle();
};

#endif

#ifndef MPU_H
#define MPU_H

#include <Arduino.h>
#include <math.h>
// ************   I2C   ************
#include <Wire.h>

class MPU{
    private:
        int read( int start, uint8_t *buffer, int size );
        int read_reg( int start, uint8_t *pbuffer );
        int write( int start, const uint8_t *pData, int size );
        int write_reg( int reg, uint8_t data );
    public:
        void init_sensor( int sda = 12, int scl = 14 );
        void update( float *pPitch, float *pGyro );
};

#endif
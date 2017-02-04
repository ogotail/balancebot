// =============================================================================
// ===                    Module for the MPY6050 6DOF IMU                    ===
// =============================================================================

#include "MPU.h"

// variables
int error ;
int a_scale ;
double g_scale ;
#define MPU6050_I2C_ADDRESS 104

//#define DEBUG_MPU

// Declaring an union for the registers and the axis values.
// The byte order does not match the byte order of
// the compiler and AVR chip.
// The AVR chip (on the Arduino board) has the Low Byte
// at the lower address.
// But the MPU-6050 has a different order: High Byte at
// lower address, so that has to be corrected.
// The register part "reg" is only used internally,
// and are swapped in code.
union accel_t_gyro_union{
    struct{
        uint8_t x_accel_h;
        uint8_t x_accel_l;
        uint8_t y_accel_h;
        uint8_t y_accel_l;
        uint8_t z_accel_h;
        uint8_t z_accel_l;
        uint8_t t_h;
        uint8_t t_l;
        uint8_t x_gyro_h;
        uint8_t x_gyro_l;
        uint8_t y_gyro_h;
        uint8_t y_gyro_l;
        uint8_t z_gyro_h;
        uint8_t z_gyro_l;
    } reg;
    struct{
        int16_t x_accel;
        int16_t y_accel;
        int16_t z_accel;
        int16_t temperature;
        int16_t x_gyro;
        int16_t y_gyro;
        int16_t z_gyro;
    } value;
};

int MPU::read( int start, uint8_t *buffer, int size ){
    int i, n ;
    Wire.beginTransmission( MPU6050_I2C_ADDRESS );
    n = Wire.write( start );
    if ( n != 1 ) return ( -10 );
    n = Wire.endTransmission( false );    // hold the I2C-bus
    if (n != 0) return (n);
    // Third parameter is true: relase I2C-bus after data is read.
    Wire.requestFrom( MPU6050_I2C_ADDRESS, size, 1 );
    i = 0 ;
    while( Wire.available() && i < size ) {
        buffer[ i++ ] = Wire.read() ;
    }
    if ( i != size ) return ( -11 );
    return ( 0 );  // return : no error
}

int MPU::read_reg( int start, uint8_t *pbuffer ){
    return read( start, pbuffer, 1 );
}

int MPU::write( int start, const uint8_t *pData, int size ){
    int n, error;
    Wire.beginTransmission(MPU6050_I2C_ADDRESS);
    n = Wire.write(start);        // write the start address
    if (n != 1) return (-20);
    n = Wire.write(pData, size);  // write data bytes
    if (n != size) return (-21);
    error = Wire.endTransmission(true); // release the I2C-bus
    if (error != 0) return (error);
    return (0);         // return : no error
}

int MPU::write_reg( int reg, uint8_t data ){
    return write( reg, &data, 1 );
}

void MPU::init_sensor( int sda, int scl ){
    byte buf_read = 0x00 ;
    Wire.begin( sda, scl );
    Wire.setClock(400000);
    // test connection
    error = read_reg( 0x75, &buf_read );
    #ifdef DEBUG_MPU
        Serial.begin();
        Serial.print( F( "connection, error = " ));
        Serial.println( error, DEC );
        Serial.print( F( "chip_id = " ));
        Serial.println( buf_read, HEX );
    #endif
    // wake it up
    error = read_reg( 0x6B, &buf_read );
    #ifdef DEBUG_MPU
        Serial.print( F( "PWR_MGMT_1 : " ));
        Serial.println( buf_read, HEX );
        Serial.print( F( "wake it up, error = " ));
        Serial.println( error, DEC );
    #endif
    // Clear the 'sleep' bit to start the sensor.
    write_reg( 0x6B, 0 );
    // set accelerometer range
    // range values 0x00, 0x08, 0x10, 0x18
    write_reg( 0x1C, 0x08 );
    // get accelerometer range
    error = read_reg( 0x1C, &buf_read );
    int scale_a[] = { 16384, 8192, 4096, 2048 };
    a_scale = scale_a[ buf_read / 8 ];
    #ifdef DEBUG_MPU
        Serial.print( F( "get accel range, error = " ));
        Serial.println( error, DEC );
    #endif
    // set gyro range
    // range values 0x00, 0x08, 0x10, 0x18
    write_reg( 0x1B, 0x00 );
    // get gyro range
    error = read_reg( 0x1B, &buf_read );
    double scale_b[] = { 131.0, 65.5, 32.8, 16.4 };
    g_scale = scale_b[ buf_read / 8 ];
    #ifdef DEBUG_MPU
        Serial.print( F( "get gyro range, error = " ));
        Serial.println( error, DEC );
    #endif
}

void MPU::update( float *pPitch, float *pGyro ){
    double x, z, angle, dT ;
    accel_t_gyro_union accel_t_gyro ;
    // get accel and gyro values
    error = read( 0x3B, (uint8_t *) &accel_t_gyro, sizeof( accel_t_gyro ));
    #ifdef DEBUG_MPU
        Serial.print( F( "get accel and gyro values, error = " ));
        Serial.println( error, DEC );
    #endif

    // Swap all high and low bytes.
    // After this, the registers values are swapped,
    // so the structure name like x_accel_l does no
    // longer contain the lower byte.
    uint8_t swap;
    #define SWAP( x, y ) swap = x; x = y; y = swap

    SWAP( accel_t_gyro.reg.x_accel_h, accel_t_gyro.reg.x_accel_l );
    SWAP( accel_t_gyro.reg.z_accel_h, accel_t_gyro.reg.z_accel_l );
    SWAP( accel_t_gyro.reg.y_gyro_h, accel_t_gyro.reg.y_gyro_l );

    // calcule le pitch
    float pitch = atan2(
        (float)accel_t_gyro.value.x_accel,
        (float)accel_t_gyro.value.z_accel
    );
    pitch = 57.2957795 *( 3.14159 + pitch ); // Rad => Deg
    if ( pitch > 180 ) pitch -= 360 ; // from -180 to 180

    *pPitch = pitch ;
    *pGyro = (float)accel_t_gyro.value.y_gyro ;

    #ifdef DEBUG_MPU
        Serial.print( pitch );
        Serial.print(F(", "));

        // Print the raw acceleration values
        Serial.print( accel_t_gyro.value.x_accel, DEC );
        Serial.print(F(", "));
        Serial.print( accel_t_gyro.value.z_accel, DEC );
        Serial.print(F(", "));
        Serial.println( accel_t_gyro.value.y_gyro, DEC );
    #endif
}

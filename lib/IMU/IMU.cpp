
// ================================================================
// ===                            IMU                           ===
// ================================================================

#include "IMU.h"


//************   IMU   ************
// I2C
#define IMU_SDA 10
#define IMU_SCL 9
// interuption
#define IMU_INT 14

//************   Variables   ************

MPU6050 mpu;

// status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
//uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU

// MPU control
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

//************   Creation   ************

IMU::IMU(){
    // join I2C bus (I2Cdev library doesn't do this automatically)
    Wire.begin( IMU_SDA, IMU_SCL );

    // initialize device
    mpu.initialize();

    // verify connection
    mpu.testConnection();

    // load and configure the DMP
    devStatus = mpu.dmpInitialize();

    // Setup Accel offsets
    mpu.setXAccelOffset(-7029);
    mpu.setYAccelOffset(-1294);
    mpu.setZAccelOffset(1200);

    // Setup gyro offsets
    mpu.setXGyroOffset(147);
    mpu.setYGyroOffset(72);
    mpu.setZGyroOffset(25);

    // make sure it worked (returns 0 if so)
    if (devStatus == 0){
        // turn on the DMP, now that it's ready
        mpu.setDMPEnabled(true);

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    }
    /*nothing to create*/
}

//************   Destruction   ************
IMU::~IMU(){
    /*nothing to destruct*/
}

//************   Initialisation   ************
void IMU::Initialisation(){
}

//************   Mise a jour des angles   ************

float IMU::Angle(){
    if ( dmpReady ){
        // get current FIFO count
        fifoCount = mpu.getFIFOCount();

        // verifie si un packet est present
        // A CHANGER !!! pour utiliser les interuptions ( if => while )
        if ( fifoCount < packetSize) return 0;
        else {
            // read all packet from FIFO until the last
            while (fifoCount >= packetSize){
                mpu.getFIFOBytes(fifoBuffer, packetSize);
                fifoCount = mpu.getFIFOCount();
            }
            // nettoie les miettes
            if (fifoCount != 0) mpu.resetFIFO();

            // Transformation geometrique
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

            // et renvoie l angle par rapport la vertical en degree
            return (ypr[1] * 180/M_PI);
        }
    }
    else return 0;
}

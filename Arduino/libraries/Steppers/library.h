#ifndef STEPPERS_H
#define STEPPERS_H

#include <stdlib.h>
#include <cstdint>
#include <rpcndr.h>

class Steppers {
protected:
    typedef enum {
        CCW     = false,            //Counter-Clockwise
        CW      = true              //Clockwise
    } Direction;
    typedef enum {
        IDLE    = 0,                //Motor idle (at position)
        SPDUP   = 1,                //Motor speed up (accelerating)
        SPDMAX  = 2,                //Motor at max speed
        SPDDW   = 3                 //Motor speed down (deacelerating)
    } MotorStat;
private:
    //Pins ------------------
    uint8_t     pinSig;             //Step signal pin
    uint8_t     pinDir;             //Direction setting pin
    uint8_t     pinEn;              //Enable control pin
    uint8_t     pinCCWS;            //End-Line CCW  sensor
    uint8_t     pinCWS;             //End-Line CW   sensor
    //Vars ------------------
    long        posCurrent;         //The current absolute position in steps.
    long        posTarget;          //The target position in steps. The stepper motor will go from current position to target position.
    long        speed;              //The current speed of the motor in steps per second
    uint32_t    accel;              //The acceleration used to accelerate or decelerate the motor in steps per second per second
    uint32_t    speedMax;           //The maximum permited speed in steps per second
    uint8_t     stepCheck;          //Counter for conmutation check
    boolean     elsActive;          //End-line sensors status
    boolean     signal;             //Step signal output status
    boolean     direction;          //Direction signal output status
    boolean     enableInv;          //Enable signal output status (inverted)
public:
    //Constructor -----------
    //Initialize the stepper only with the driver control
    Steppers(uint8_t signal, uint8_t direction, uint8_t enable);
    //Initialize the stepper with the driver control signals and the end-line sensors
    Steppers(uint8_t signal, uint8_t direction, uint8_t enable, uint8_t lels, uint8_t rels);
    //-----------------------
    void move(long _relative);                  //Set the target position.
    void moveTo(long _absolute);                //Set the target position relative to the current position.
    void setMaxSpeed(uint32_t _speed);
    void setAcceleration(uint32_t _accel);
    void setCurrentPosition(long _position);
    //-----------------------
    boolean updateMotor();
    boolean updateMotor
    //-----------------------

};

#endif
#ifndef HARDWARE_COMMS_H
#define HARDWARE_COMMS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    STATUS_CODE_OK                  = 0,
    STATUS_CODE_FAILURE_TIMEOUT     = 1,
    STATUS_CODE_FAILURE_INTERRUPT   = 2,
} DispenseStatusCode;

typedef 
struct {
    bool isDone;
    DispenseStatusCode statusCode;
} MotorReturnValue;

MotorReturnValue processMotorControl(uint8_t MotorControl);

#endif
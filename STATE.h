#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum{
	SYS_IDLE,
	SYS_START_MOTOR,
	SYS_DISPENSING,
	SYS_SUCCESS,
	SYS_FAILURE
} SystemState;


volatile extern SystemState motorState;	// Global state variable
volatile extern uint8_t motorBit;		// Motor bit selected

#endif
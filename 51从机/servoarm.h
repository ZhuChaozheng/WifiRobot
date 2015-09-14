#ifndef __SERVOARM_H__
#define __SERVOARM_H__
#include "C8051F360.h"

#define TIME_GAP_20US

#ifdef TIME_GAP_20US
	#define  POS_MOD_MAX 120
	#define  POS_MOD_MIN 26
#else
	#define POS_MOD_MAX 240
	#define POS_MOD_MIN 52
#endif

#define SERVO_MAX 0
#define SERVO_MID 1
#define SERVO_MIN 2



sbit servo_pin =  P1^7;


void servo_init(void);

void servo_port_init(void);

void servo_turn_left();

void servo_turn_right();

void servo_stop();


#endif
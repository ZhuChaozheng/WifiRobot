#ifndef    _PWM_H_
#define    _PWM_H_

#include"c8051f360.h"


#define SYSCLK       24500000

void DC_Motor(unsigned char motor_num,
              unsigned char direction, unsigned char motor_speed);
//void PWM_Load_Value(void);
void PWM_Port_Init(void);
//void Oscillator_Init (void);
void PWM_Init(void);


#endif
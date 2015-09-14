#include "servoarm.h"
#include "Timer2.h"
unsigned int servo_pwm = 0;
/*
const unsigned char servo_max[3] = {162/2,200/2,190/2};
const unsigned char servo_mid[3] = {138/2,145/2,145/2};
const unsigned char servo_min[3] = {110/2,115/2,100/2};
*/

void servo_init(void)
{
	servo_port_init();
	Timer2_Init();
	servo_stop();
}

void servo_port_init(void)
{
    unsigned char SFRPAGE_save = SFRPAGE; 
	SFRPAGE = CONFIG_PAGE; 

	P1SKIP |= 0x80;
    P1MDOUT |= 0x80;    //P1.7

//	XBR1 |= 0x40;

    SFRPAGE = SFRPAGE_save; 
}



void servo_turn_left()
{
	servo_pwm = 144/2;
}

void servo_turn_right()
{
	servo_pwm = 152/2;
}

void servo_stop()
{
	servo_pwm = 148/2;
}


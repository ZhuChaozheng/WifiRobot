#include "Timer2.h"
#include "C8051F360.h"
extern unsigned int servo_pwm;


void Timer2_Init()
{
	unsigned char SFRPAGE_save;
	CKCON  |= 0x10;    //系统时钟

	SFRPAGE_save = SFRPAGE;
	SFRPAGE = TMR2_PAGE;
	

#ifdef TIME_GAP_20US
    TMR2CN  = 0x05;	   //16位,允许中断
	TMR2RLL = 0x15;	   
	TMR2RLH = 0xfe;	      //20us
#else
	TMR2CN  = 0x29;	   //低八位，允许中断
    TMR2RLL = 0x0a;       //10us
#endif
	SFRPAGE = SFRPAGE_save; 

	IE |= 0x20;

} 

data unsigned int i=0, SERVO_PWM_C=0;
void Timer2_ISR() interrupt INTERRUPT_TIMER2
{
  
    
    
	//static unsigned char past;
	TMR2CN &= 0x3f;
	i++;

	if(i > SERVO_PWM_C)
	{
	    //servo_pin = 0;
		servo_pin = 0;
		SERVO_PWM_C = 1999;
	}


	if(i > 999)
	{
	    i = 0;
		servo_pin = 1;
		SERVO_PWM_C = servo_pwm;
	}

}







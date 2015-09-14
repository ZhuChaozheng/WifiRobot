#ifndef    _SENSOR_AD_
#define    _SENSOR_AD_

#include "c8051f360.h"


/****************************************************************
//! Declaration functions used in Sensor_AD.c
****************************************************************/
void ADC_PortInit(void);
void ADC_Init(void);
void Oscillator_Init (void);
void ADC_ConvertStart(void);
void  AD_PortSelection(unsigned char AD_port);
unsigned int ADC_Get(unsigned char AD_port);
unsigned char ADC_IR_Get(unsigned char AD_port);
unsigned int ADC_AvrDataGet(unsigned int * AD_Array);
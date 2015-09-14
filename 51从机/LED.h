#ifndef __led_H__
#define __led_H__


void led(unsigned char i);
void led_init();
void led_disapper(unsigned char i);

sbit LEDEN0     = P0^2;
sbit LEDEN1     = P0^3;
sbit LEDEN2     = P0^4;
sbit LEDEN3     = P0^5;
sbit LEDEN4     = P0^6;
sbit LEDEN5     = P0^7;
sbit LEDEN6     = P1^0;
sbit LEDEN7     = P1^1;
sbit LEDEN8     = P1^2;
sbit LEDEN9     = P1^3;
sbit LEDEN10     = P3^4;
sbit LEDEN11     = P3^5;
sbit LEDEN12     = P3^6;
sbit LEDEN13     = P3^7;

#endif 
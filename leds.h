#ifndef __LEDS_H__
#define __LEDS_H__

/******************************************************************************
 *  Copyright MindMe 2015
 
 *  FILE
 *      leds.c
 *
 *  DESCRIPTION
 *      Miscellaneous LED control routines.
 *
 ******************************************************************************/

#include <types.h>

void setLEDs(bool on);
void DisablePWMs(void);
void initPWMs(void);
extern void SetLEDColor(uint8 red, uint8 green, uint8 blue);
bool setPWMDuty(uint16 pwm_id, uint8 duty);

#endif
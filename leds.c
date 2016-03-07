/******************************************************************************
 *  Copyright MindMe 2015
 
 *  FILE
 *      leds.c
 *
 *  DESCRIPTION
 *      Miscellaneous LED control routines.
 *
 ******************************************************************************/
#include <pio.h>

#include "leds.h"
#include "hw_config.h"

//static uint8 RedDuty;
//static uint8 GreenDuty;
//static uint8 BlueDuty;

#define PWM_PERIOD      255
#define PWM_HOLD_TIME   0

/* Colour depth in bits, as passed by application. */
#define LIGHT_INPUT_COLOR_DEPTH  (8)
/* Colour depth in bits, mapped to actual hardware. */
#define LIGHT_MAPPED_COLOR_DEPTH (6)
/* Colour depth lost due to re-quantization of levels. */
#define QUANTIZATION_ERROR       (LIGHT_INPUT_COLOR_DEPTH -\
                                  LIGHT_MAPPED_COLOR_DEPTH)
/* Maximum colour level supported by mapped colour depth bits. */
#define COLOR_MAX_VALUE          ((0x1 << LIGHT_MAPPED_COLOR_DEPTH) - 1)

void initPWMs(void) {
    PioSetDirs(LED_PIO_MASK, LED_PIO_MASK);         // set to outputs

    setPWMDuty(PWM_RED, 0);
    setPWMDuty(PWM_GREEN, 0);
    setPWMDuty(PWM_BLUE, 0);
    PioSetMode(LED_RED, pio_mode_pwm0);
    PioSetMode(LED_GREEN, pio_mode_pwm1);
    PioSetMode(LED_BLUE, pio_mode_pwm2);
    PioEnablePWM(PWM_RED, TRUE);
    PioEnablePWM(PWM_GREEN, TRUE);
    PioEnablePWM(PWM_BLUE, TRUE);
}

void DisablePWMs(void) {
    PioSetDirs(LED_PIO_MASK, LED_PIO_MASK);
    PioSetMode(LED_RED, pio_mode_user);
    PioSetMode(LED_GREEN, pio_mode_user);
    PioSetMode(LED_BLUE, pio_mode_user);
    PioSet(LED_RED, TRUE);
    PioSet(LED_GREEN, TRUE);
    PioSet(LED_BLUE, TRUE);
}
            
bool setPWMDuty(uint16 pwm_id, uint8 duty) {
    return PioConfigPWM(pwm_id, pio_pwm_mode_push_pull,
                            PWM_PERIOD - duty, duty, PWM_HOLD_TIME,
                            PWM_PERIOD - duty, duty, PWM_HOLD_TIME,
                            0);
}

extern void SetLEDColor(uint8 red, uint8 green, uint8 blue) {
    if ((red >> QUANTIZATION_ERROR) == 0)
    {
         PioSetMode(LED_RED, pio_mode_user);
         PioSet(LED_RED, 1);
    }
    else
    {
        PioSetMode(LED_RED, pio_mode_pwm0);
    }

    if ((green >> QUANTIZATION_ERROR) == 0)
    {
         PioSetMode(LED_GREEN, pio_mode_user);
         PioSet(LED_GREEN, 1);
    }
    else
    {
        PioSetMode(LED_GREEN, pio_mode_pwm1);
    }

    if ((blue >> QUANTIZATION_ERROR) == 0)
    {
         PioSetMode(LED_BLUE, pio_mode_user);
         PioSet(LED_BLUE, 1);
    }
    else
    {
        PioSetMode(LED_BLUE, pio_mode_pwm2);
    }

    /* Invert values as its a pull down */
    red   = COLOR_MAX_VALUE - (red >> QUANTIZATION_ERROR);
    green = COLOR_MAX_VALUE - (green >> QUANTIZATION_ERROR);
    blue  = COLOR_MAX_VALUE - (blue >> QUANTIZATION_ERROR);

    PioConfigPWM(PWM_RED, pio_pwm_mode_push_pull,
                 red, (COLOR_MAX_VALUE - red), 1U,
                 red, (COLOR_MAX_VALUE - red), 1U, 0U);
    PioConfigPWM(PWM_GREEN, pio_pwm_mode_push_pull,
                 green, (COLOR_MAX_VALUE - green), 1U,
                 green, (COLOR_MAX_VALUE - green), 1U, 0U);
    PioConfigPWM(PWM_BLUE, pio_pwm_mode_push_pull,
                 blue, (COLOR_MAX_VALUE - blue), 1U,
                 blue, (COLOR_MAX_VALUE - blue), 1U, 0U);   
}

void setLEDs(bool on) {
    PioSetDirs(LED_PIO_MASK, LED_PIO_MASK);
    if (on) PioSets(LED_PIO_MASK, 0);
    else PioSets(LED_PIO_MASK, LED_PIO_MASK);
}
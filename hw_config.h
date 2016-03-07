#ifndef __HW_CONFIG_H__
#define __HW_CONFIG_H__

#include <types.h>
#include "hw_access.h"      // Includes PIO_BIT_MASK helper macro

/* Setup PIO 11 as Button PIO */
#define BUTTON_PIO                  (3)

#define BUTTON_PIO_MASK             (PIO_BIT_MASK(BUTTON_PIO))

/* LEDs are on PIO9:11 */
#define LED_PIO_MASK                (0x00000E00UL)

#define LED_RED             (9)
#define LED_GREEN           (10)
#define LED_BLUE            (11)

#define PWM_RED             (0)
#define PWM_GREEN           (1)
#define PWM_BLUE            (2)

#endif
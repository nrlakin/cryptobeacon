/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      buzzer.h
 *
 *  DESCRIPTION
 *      This file contains prototypes for accessing buzzer functionality.
 *
 *****************************************************************************/

#ifndef __BUZZER_H__
#define __BUZZER_H__

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "user_config.h"    /* User configuration */

/* Only compile this file if the buzzer code has been requested */
#ifdef ENABLE_BUZZER

/*============================================================================*
 *  Public data type
 *============================================================================*/

/* Data type for different type of buzzer beeps */
typedef enum
{
    /* No beeps */
    buzzer_beep_off = 0,

    /* Short beep */
    buzzer_beep_short,

    /* Long beep */
    buzzer_beep_long,

    /* Two short beeps */
    buzzer_beep_twice,

    /* Three short beeps */
    buzzer_beep_thrice

} buzzer_beep_type;

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Initialise the buzzer hardware */
extern void BuzzerInitHardware(void);

/* Initialise the buzzer data to a known state */
extern void BuzzerInitData(void);

/* Reset the buzzer data to a clean state */
extern void BuzzerResetData(void);

/* Trigger beeps of different types, enumerated by 'buzzer_beep_type' */
extern void SoundBuzzer(buzzer_beep_type beep_type);

#else /* ENABLE_BUZZER */

/* Define buzzer functions to expand to nothing as buzzer functionality is not 
 * enabled 
 */

#define BuzzerInitHardware()
#define BuzzerInitData()
#define BuzzerResetData()
#define SoundBuzzer(beep_type)

#endif /* ENABLE_BUZZER */

#endif /* __BUZZER_H__ */


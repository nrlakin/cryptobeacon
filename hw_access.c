/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      hw_access.c
 *
 *  DESCRIPTION
 *      This file defines the application hardware specific routines.
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <pio.h>            /* PIO configuration and control functions */
#include <pio_ctrlr.h>      /* Access to the PIO controller */
#include <timer.h>          /* Chip timer functions */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "hw_access.h"      /* Interface to this file */
#include "hw_config.h"      /* PIO definitions */
#include "cryptobeacon.h"    /* Definitions used throughout the GATT server */
#include "leds.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Extra long button press timer */
#define EXTRA_LONG_BUTTON_PRESS_TIMER \
                                    (4*SECOND)

/*============================================================================*
 *  Public data type
 *============================================================================*/

/* Application Hardware data structure */
typedef struct _APP_HW_DATA_T
{

    /* Timer for button press */
    timer_id                    button_press_tid;

} APP_HW_DATA_T;

/*============================================================================*
 *  Public data
 *============================================================================*/

/* Application hardware data instance */
static APP_HW_DATA_T            g_app_hw_data;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Handle extra long button press */
static void handleExtraLongButtonPress(timer_id tid);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleExtraLongButtonPress
 *
 *  DESCRIPTION
 *      This function contains handling of extra long button press, which
 *      triggers pairing / bonding removal.
 *
 *  PARAMETERS
 *      tid [in]                ID of timer that has expired
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleExtraLongButtonPress(timer_id tid)
{
    if(tid == g_app_hw_data.button_press_tid)
    {
        /* Re-initialise button press timer */
        g_app_hw_data.button_press_tid = TIMER_INVALID;

        /* Sound three beeps to indicate pairing removal to user */
        // SoundBuzzer(buzzer_beep_thrice);

        /* Handle pairing removal */
        //HandlePairingRemoval();

    } /* Else ignore timer */

}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

extern void IndicateAdvertisingServer(void) {
    SetLEDColor(20, 20, 0);     // sort of orange
}

extern void IndicateConnection(void) {
    SetLEDColor(0, 400, 0);     // green
}

extern void IndicateUnhandledState(void) {
    SetLEDColor(5, 0, 0);
}

extern void ReadyHWForSleep(void) {
    DisablePWMs();
    //PioSetModes(LED_PIO_MASK, pio_mode_user);
    //PioSetDirs(LED_PIO_MASK, FALSE);
    //PioSetPullModes(LED_PIO_MASK, pio_mode_strong_pull_up);
    //SetLEDColor(0, 0, 0);
}
    
/*----------------------------------------------------------------------------*
 *  NAME
 *      InitHardware
 *
 *  DESCRIPTION
 *      This function is called to initialise the application hardware.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void InitHardware(void)
{
    /* Setup PIOs
     * PIO3 - Button
     * PIO9:11 - LEDs
     */
    /* First, set strong pulldowns as default. */
    PioSetModes(0xFFFFFFFFUL, pio_mode_user);
    PioSetDirs(0xFFFFFFFFUL, FALSE);
    PioSetPullModes(0xFFFFFFFFUL, pio_mode_strong_pull_down);

    /* Set the Button PIO direction as input */
    PioSetDir(BUTTON_PIO, PIO_DIRECTION_INPUT);

    /* Pull up the PIO */
    PioSetPullModes(BUTTON_PIO_MASK, pio_mode_strong_pull_up);

    /* Request an event when the button PIO changes state */
    PioSetEventMask(BUTTON_PIO_MASK, pio_event_mode_both);

    /* Set LEDs as outputs, default high (LED off) */
    //PioSetDirs(0x00000E00UL, TRUE);     // LEDs are set to output high (off)
    //PioSets(0x00000E00UL, 0x00000E00UL);
    //PioSetPullModes(0x00000E00UL, pio_mode_no_pulls);
    initPWMs();
    
    /* Save power by changing the I2C pull mode to pull down.*/
    PioSetI2CPullMode(pio_i2c_pull_mode_strong_pull_down);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HwDataInit
 *
 *  DESCRIPTION
 *      This function initialises the hardware data to a known state. It is
 *      intended to be called once, for example after a power-on reset or HCI
 *      reset.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void HwDataInit(void)
{
    /* Initialise button press timer */
    g_app_hw_data.button_press_tid = TIMER_INVALID;

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HwDataReset
 *
 *  DESCRIPTION
 *      This function resets the hardware data. It is intended to be called when
 *      the data needs to be reset to a clean state, for example, whenever a
 *      device connects or disconnects.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void HwDataReset(void)
{
    /* Delete button press timer */
    if (g_app_hw_data.button_press_tid != TIMER_INVALID)
    {
        TimerDelete(g_app_hw_data.button_press_tid);
        g_app_hw_data.button_press_tid = TIMER_INVALID;
    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandlePIOChangedEvent
 *
 *  DESCRIPTION
 *      This function handles the PIO Changed event.
 *
 *  PARAMETERS
 *      pio_data [in]           State of the PIOs when the event occurred
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void HandlePIOChangedEvent(pio_changed_data *pio_data)
{

    if(pio_data->pio_cause & BUTTON_PIO_MASK)
    {
        /* PIO changed */
        uint32 pios = PioGets();

        if(!(pios & BUTTON_PIO_MASK))
        {
            /* This event is triggered when a button is pressed. */

            /* Start a timer for EXTRA_LONG_BUTTON_PRESS_TIMER seconds. If the
             * timer expires before the button is released an extra long button
             * press is detected. If the button is released before the timer
             * expires a short button press is detected.
             */
            TimerDelete(g_app_hw_data.button_press_tid);

            g_app_hw_data.button_press_tid =
                TimerCreate(EXTRA_LONG_BUTTON_PRESS_TIMER,
                                           TRUE, handleExtraLongButtonPress);
        }
        else
        {
            /* This event comes when a button is released. */
            if(g_app_hw_data.button_press_tid != TIMER_INVALID)
            {
                /* Timer was already running. This means it was a short button
                 * press.
                 */
                TimerDelete(g_app_hw_data.button_press_tid);
                g_app_hw_data.button_press_tid = TIMER_INVALID;

                /* Indicate short button press using short beep */
                //SoundBuzzer(buzzer_beep_short);

                HandleShortButtonPress();
            }
        }
    }
}

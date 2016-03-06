/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      hw_access.h
 *
 *  DESCRIPTION
 *      Header definitions for HW setup.
 *
 *****************************************************************************/

#ifndef __HW_ACCESS_H__
#define __HW_ACCESS_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <sys_events.h>     /* System Event definitions and declarations */

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Convert a PIO number into a bit mask */
#define PIO_BIT_MASK(pio)       (0x01UL << (pio))

/* PIO direction */
#define PIO_DIRECTION_INPUT     (FALSE)
#define PIO_DIRECTION_OUTPUT    (TRUE)

/* PIO state */
#define PIO_STATE_HIGH          (TRUE)
#define PIO_STATE_LOW           (FALSE)

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Initialise the application hardware */
extern void InitHardware(void);

/* Initialise the application hardware data structure */
extern void HwDataInit(void);

/* Reset the application hardware data structure */
extern void HwDataReset(void);

/* Handle the PIO changed event */
extern void HandlePIOChangedEvent(pio_changed_data *pio_data);

#endif /* __HW_ACCESS_H__ */

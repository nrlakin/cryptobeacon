/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      gatt_server.h
 *
 *  DESCRIPTION
 *      Header file for a simple GATT server application.
 *
 ******************************************************************************/

#ifndef __GATT_SERVER_H__
#define __GATT_SERVER_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>          /* Commonly used type definitions */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "gatt_access.h"    /* GATT-related routines */

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Maximum number of words in central device Identity Resolving Key (IRK) */
#define MAX_WORDS_IRK                       (8)

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Call the firmware Panic() routine and provide a single point for debugging
 * any application level panics
 */
extern void ReportPanic(app_panic_code panic_code);

/* Handle a short button press. If connected, the device disconnects from the
 * host, otherwise it starts advertising.
 */
extern void HandleShortButtonPress(void);

/* Change the current state of the application */
extern void SetState(app_state new_state);

/* Return the current state of the application.*/
extern app_state GetState(void);

/* Check if the whitelist is enabled or not. */
extern bool IsWhiteListEnabled(void);

/* Handle pairing removal */
extern void HandlePairingRemoval(void);

/* Start the advertisement timer. */
extern void StartAdvertTimer(uint32 interval);

/* Return whether the connected device is bonded or not */
extern bool IsDeviceBonded(void);

/* Return the unique connection ID (UCID) of the connection */
extern uint16 GetConnectionID(void);

#endif /* __GATT_SERVER_H__ */

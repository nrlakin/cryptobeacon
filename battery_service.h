/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      battery_service.h
 *
 *  DESCRIPTION
 *      Header definitions for the Battery Service
 *
 *****************************************************************************/

#ifndef __BATTERY_SERVICE_H__
#define __BATTERY_SERVICE_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>          /* Commonly used type definitions */
#include <gatt.h>           /* GATT application interface */

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Initialise the Battery Service data structure.*/
extern void BatteryDataInit(void);

/* Initialise the Battery Service data structure at chip reset */
extern void BatteryInitChipReset(void);

/* Handle read operations on Battery Service attributes maintained by the
 * application
 */
extern void BatteryHandleAccessRead(GATT_ACCESS_IND_T *p_ind);

/* Handle write operations on Battery Service attributes maintained by the
 * application
 */
extern void BatteryHandleAccessWrite(GATT_ACCESS_IND_T *p_ind);

/* Monitor the battery level and trigger notifications (if configured) to the
 * connected host
 */
extern void BatteryUpdateLevel(uint16 ucid);

/* Read the Battery Service specific data stored in NVM */
extern void BatteryReadDataFromNVM(uint16 *p_offset);

/* Check if the handle belongs to the Battery Service */
extern bool BatteryCheckHandleRange(uint16 handle);

/* Notify bonding status to the Battery Service */
extern void BatteryBondingNotify(void);

#endif /* __BATTERY_SERVICE_H__ */

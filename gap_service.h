/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      gap_service.h
 *
 *  DESCRIPTION
 *      Header definitions for the GAP Service
 *
 *****************************************************************************/

#ifndef __GAP_SERVICE_H__
#define __GAP_SERVICE_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>          /* Commonly used type definitions */
#include <gatt.h>           /* GATT application interface */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "gap_conn_params.h"/* Connection and advertisement definitions */

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Initialise the GAP Service data structure.*/
extern void GapDataInit(void);

/* Handle read operations on GAP Service attributes maintained by the
 * application
 */
extern void GapHandleAccessRead(GATT_ACCESS_IND_T *p_ind);

/* Handle write operations on GAP Service attributes maintained by the
 * application
 */
extern void GapHandleAccessWrite(GATT_ACCESS_IND_T *p_ind);

/* Read the GAP Service specific data stored in NVM */
extern void GapReadDataFromNVM(uint16 *p_offset);

/* Write GAP Service specific data to NVM for the first time during
 * application initialisation
 */
extern void GapInitWriteDataToNVM(uint16 *p_offset);

/* Check if the handle belongs to the GAP Service */
extern bool GapCheckHandleRange(uint16 handle);

/* Get the reference to the 'g_device_name' array, which contains AD Type and
 * device name
 */
extern uint8 *GapGetNameAndLength(uint16 *p_name_length);

#endif /* __GAP_SERVICE_H__ */

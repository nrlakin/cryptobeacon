/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      debug_interface.h
 *
 *  DESCRIPTION
 *      This file defines debug output routines for the application
 *
 *****************************************************************************/

#ifndef __DEBUG_INTERFACE_H__
#define __DEBUG_INTERFACE_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <bluetooth.h>      /* Bluetooth specific type definitions */
#include <debug.h>          /* Simple host interface to the UART driver */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "user_config.h"    /* User configuration */

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

#ifdef DEBUG_OUTPUT_ENABLED

/* Map macros to the debug functions */
#define DebugIfWriteString(a)          DebugWriteString(a)
#define DebugIfWriteUint8(a)           DebugWriteUint8(a)
#define DebugIfWriteUint16(a)          DebugWriteUint16(a)
#define DebugIfWriteUint32(a)          DebugWriteUint32(a)

/*----------------------------------------------------------------------------*
 *  NAME
 *      DebugIfInit
 *
 *  DESCRIPTION
 *      Initialise debug output.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void DebugIfInit(void);

/*----------------------------------------------------------------------------*
 *  NAME
 *      DebugIfWriteBdAddress
 *
 *  DESCRIPTION
 *      Print out a Bluetooth address.
 *
 *  PARAMETERS
 *      address [in]            Bluetooth address to write to UART
 *
 *  RETURNS
 *      Nothing
 *---------------------------------------------------------------------------*/
extern void DebugIfWriteBdAddress(const TYPED_BD_ADDR_T *address);

/*----------------------------------------------------------------------------*
 *  NAME
 *      DebugIfWriteInt
 *
 *  DESCRIPTION
 *      Print out an integer value
 *
 *  PARAMETERS
 *      value [in]              Integer value to write to UART
 *
 *  RETURNS
 *      Nothing
 *---------------------------------------------------------------------------*/
extern void DebugIfWriteInt(int16 value);

/* Write an error message with a status code */
#define DebugIfWriteErrorMessage(msg, error)   \
    DebugWriteString(msg);                     \
    DebugWriteString(" (0x");                  \
    DebugWriteUint16((uint16)error);           \
    DebugWriteString(")\r\n")

/*----------------------------------------------------------------------------*
 *  NAME
 *      DebugIfWriteUuid128
 *
 *  DESCRIPTION
 *      Print out a 128-bit UUID
 *
 *  PARAMETERS
 *      uuid [in]               Buffer containing 128-bit UUID to write to UART
 *
 *  RETURNS
 *      Nothing
 *---------------------------------------------------------------------------*/
extern void DebugIfWriteUuid128(const uint8 *uuid);
    
#else

#define DebugIfWriteString(a)
#define DebugIfWriteUint8(a)
#define DebugIfWriteUint16(a)
#define DebugIfWriteUint32(a)
#define DebugIfInit()
#define DebugIfWriteBdAddress(a)
#define DebugIfWriteInt(a)
#define DebugIfWriteErrorMessage(msg, error)
#define DebugIfWriteUuid128(a)

#endif /* DEBUG_OUTPUT_ENABLED */

#endif /* __DEBUG_INTERFACE_H__ */


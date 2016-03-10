/*******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 * FILE
 *      user_config.h
 *
 * DESCRIPTION
 *      This file contains definitions to customise the application.
 *
 ******************************************************************************/

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

/*=============================================================================*
 *  Public Definitions
 *============================================================================*/

/* The PAIRING_SUPPORT macro controls whether pairing and encryption code is
 * compiled. This flag may be disabled for the applications that do not require
 * pairing.
 */

#define IBEACON_MAJOR_REGION    0xBBBB
#define IBEACON_MINOR_REGION    0xCCCC

/* This macro when defined enables the debug output on UART */
//#define DEBUG_OUTPUT_ENABLED

/* The CONNECTED_IDLE_TIMEOUT_VALUE macro specifies how long the application may
 * be idle for during the Connected state. The device will disconnect when this
 * timer expires. Vendors are free to decide whether to enable this timeout, and
 * the timeout duration, as per their use case.
 * If this macro is not defined there will be no disconnection.
 */
/*#define CONNECTED_IDLE_TIMEOUT_VALUE   (5 * MINUTE)*/

#endif /* __USER_CONFIG_H__ */

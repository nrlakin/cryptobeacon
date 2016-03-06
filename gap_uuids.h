/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      gap_uuids.h
 *
 *  DESCRIPTION
 *      UUID MACROs for the GAP Service
 *
 *****************************************************************************/

#ifndef __GAP_UUIDS_H__
#define __GAP_UUIDS_H__

/*============================================================================*
 *         Public Definitions
 *============================================================================*/

/* Brackets should not be used around the values of these macros. This file is
 * imported by the GATT Database Generator (gattdbgen) which does not understand 
 * brackets and will raise syntax errors.
 */

/* For UUID values, refer http://developer.bluetooth.org/gatt/services/
 * Pages/ServiceViewer.aspx?u=org.bluetooth.service.generic_access.xml
 */

#define UUID_GAP                                       0x1800

#define UUID_DEVICE_NAME                               0x2A00

#define UUID_APPEARANCE                                0x2A01

#define UUID_PER_PREF_CONN_PARAMS                      0x2A04

#endif /* __GAP_UUIDS_H__ */

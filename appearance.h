/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 * FILE
 *     appearance.h
 *
 *  DESCRIPTION
 *     This file defines macros for commonly used appearance values, which are 
 *     defined by BT SIG.
 *
 *****************************************************************************/

#ifndef __APPEARANCE_H__
#define __APPEARANCE_H__

/*=====================================================*
 *  Public Definitions
 *=====================================================*/

/* Brackets should not be used around the values of these macros. This file is
 * imported by the GATT Database Generator (gattdbgen) which does not understand 
 * brackets and will raise syntax errors.
 */

/* For values, refer http://developer.bluetooth.org/gatt/characteristics/Pages/
 * CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.appearance.xml
 */

/* Unknown appearance value */
#define APPEARANCE_UNKNOWN_VALUE                0x0000

#define APPEARANCE_GATT_SERVER_VALUE            APPEARANCE_UNKNOWN_VALUE

#define APPEARANCE_APPLICATION_VALUE            APPEARANCE_GATT_SERVER_VALUE

#endif /* __APPEARANCE_H__ */

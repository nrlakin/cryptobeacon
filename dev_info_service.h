/*****************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      dev_info_service.h
 *
 *  DESCRIPTION
 *      Header definitions for the Device Information Service
 *
 *****************************************************************************/

#ifndef __DEV_INFO_SERVICE_H__
#define __DEV_INFO_SERVICE_H__

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Handle read operations on Device Information Service attributes maintained by
 * the application
 */
extern void DeviceInfoHandleAccessRead(GATT_ACCESS_IND_T *p_ind);

/* Check if the handle belongs to the Device Information Service */
extern bool DeviceInfoCheckHandleRange(uint16 handle);

#endif /*__DEVICE_INFO_SERVICE_H__*/

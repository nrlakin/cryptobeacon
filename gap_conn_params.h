/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      gap_conn_params.h
 *
 *  DESCRIPTION
 *      MACROs for connection and advertisement parameter values
 *
 *****************************************************************************/

#ifndef __GAP_CONN_PARAMS_H__
#define __GAP_CONN_PARAMS_H__

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Advertising parameters. Time is expressed in microseconds and the firmware
 * will round this down to the nearest slot. Acceptable range is 20ms to 10.24s
 * and the minimum must be no larger than the maximum. This value needs to be 
 * modified at a later stage as decided GPA for specific profile.
 *
 * For the GATT Server application, use 60ms as the fast connection
 * advertisement interval. For reduced power connections the recommended range
 * is 1s to 2.5s. Vendors will need to tune these values as per their
 * requirements.
 */
#define FC_ADVERTISING_INTERVAL_MIN         (60 * MILLISECOND)
#define FC_ADVERTISING_INTERVAL_MAX         (60 * MILLISECOND)

#define RP_ADVERTISING_INTERVAL_MIN         (1280 * MILLISECOND)
#define RP_ADVERTISING_INTERVAL_MAX         (1280 * MILLISECOND)

/* Maximum number of connection parameter update requests that can be sent when 
 * connected
 */
#define MAX_NUM_CONN_PARAM_UPDATE_REQS      (2)

/* Brackets should not be used around the values of these macros. This file is
 * imported by the GATT Database Generator (gattdbgen) which does not understand 
 * brackets and will raise syntax errors.
 */

/* Preferred connection parameter values should be within the range specified by
 * the Bluetooth specification.
 */

/* Minimum and maximum connection interval in number of frames */
#define PREFERRED_MAX_CON_INTERVAL          0x0190 /* 500 ms */
#define PREFERRED_MIN_CON_INTERVAL          0x0190 /* 500 ms */

/* Slave latency in number of connection intervals */
#define PREFERRED_SLAVE_LATENCY             0x0004 /* 4 conn_intervals */

/* Supervision timeout (ms) = PREFERRED_SUPERVISION_TIMEOUT * 10 ms */
#define PREFERRED_SUPERVISION_TIMEOUT       0x03E8 /* 10 seconds */

#endif /* __GAP_CONN_PARAMS_H__ */

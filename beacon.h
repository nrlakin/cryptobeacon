/******************************************************************************
 *  Copyright (C) Cambridge Silicon Radio Limited 2014
 *
 *  FILE
 *      beacon.h
 *
 *  DESCRIPTION
 *      Defines advertising and other functional macros
 *
 *****************************************************************************/

#ifndef _BEACON_H_
#define _BEACON_H_

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Advertising interval range, in milliseconds 
 * Minimum advertisement interval for non connectable adverts is 100ms.
 * Maximum advertisement interval for non connectable adverts is 10.24 seconds.
 */
#define MIN_ADVERTISING_INTERVAL        (100)           /* 100ms */
#define MAX_ADVERTISING_INTERVAL        (10240)         /* 10.24s */

/* Default advertising interval, time is expressed in milliseconds and the 
 * firmware will round this down to the nearest slot. Acceptable range is 100ms 
 * to 10.24s and the minimum must be no larger than the maximum.
 */
#define DEFAULT_ADVERTISING_INTERVAL    (1000)          /* 1s */

/* Maximum advertising packet size */
#define MAX_ADVERT_PACKET_SIZE          (31)

/* Maximum advertisement node payload size:
 *      31
 *      - 3 octets for mandatory Flags AD (added automatically by the firmware)
 *      - 1 octet for manufacturer specific AD length field (added by firmware)
 *      - 1 octet for the AD type
 *      - 2 octets for manufacturer id
 */
#define MAX_ADVERT_PAYLOAD_SIZE         (MAX_ADVERT_PACKET_SIZE - 3 - 1 - 1 - 2)

/* Default advertising node payload size - fill the whole advertising packet */
#define DEFAULT_ADVERT_PAYLOAD_SIZE     (MAX_ADVERT_PAYLOAD_SIZE)

extern void SetupBeaconAdPacket(void);

#endif /* _BEACON_H_ */

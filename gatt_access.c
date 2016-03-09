/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      gatt_access.c
 *
 *  DESCRIPTION
 *      GATT-related routine implementations
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <ls_app_if.h>      /* Link Supervisor application interface */
#include <gap_app_if.h>     /* GAP application interface */
#include <gap_types.h>      /* GAP definitions */
#include <ls_err.h>         /* Upper Stack Link Supervisor error codes */
#include <ls_types.h>       /* Link Supervisor definitions */
#include <panic.h>          /* Support for applications to panic */
#include <gatt.h>           /* GATT application interface */
#include <gatt_uuid.h>      /* Common Bluetooth UUIDs and macros */
#include <timer.h>          /* Chip timer functions */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "cryptobeacon.h"    /* Definitions used throughout the application */
#include "app_gatt_db.h"    /* GATT database definitions */
#include "gatt_access.h"    /* Interface to this file */
#include "appearance.h"     /* Macros for commonly used appearance values */
#include "gap_service.h"    /* GAP Service interface */
#include "battery_service.h"/* Battery Service interface */
#include "battery_uuids.h"  /* Battery Service UUIDs */
#include "dev_info_uuids.h" /* Device Information Service UUIDs */
#include "dev_info_service.h"/* Device Information Service interface */
#include "auth_uuids.h"
#include "auth_service.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* This constant defines an array that is large enough to hold the advertisement
 * data.
 */
#define MAX_ADV_DATA_LEN                                  (31)

/* Acceptable shortened device name length that can be sent in advertisement 
 * data 
 */
#define SHORTENED_DEV_NAME_LEN                            (8)

/* Length of Tx Power prefixed with 'Tx Power' AD Type */
#define TX_POWER_VALUE_LENGTH                             (2)

/*============================================================================*
 *  Private Data types
 *============================================================================*/

/* GATT data structure */
typedef struct _APP_GATT_DATA_T
{
    /* Value for which advertisement timer needs to be started. 
     *
     * For bonded devices, the timer is initially started for 10 seconds to 
     * enable fast connection by bonded device to the sensor. If bonded device 
     * doesn't connect within this time, another 20 second timer is started 
     * to enable fast connections from any collector device in the vicinity. 
     * This is then followed by reduced power advertisements.
     *
     * For non-bonded devices, the timer is initially started for 30 seconds 
     * to enable fast connections from any collector device in the vicinity.
     * This is then followed by reduced power advertisements.
     */
    uint32                     advert_timer_value;

} APP_GATT_DATA_T;

/*============================================================================*
 *  Private Data 
 *============================================================================*/

/* Application GATT data instance */
static APP_GATT_DATA_T g_gatt_data;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Add device name to advertisement or scan response */
static void addDeviceNameToAdvData(uint16 adv_data_len, uint16 scan_data_len);

/* Set advertisement parameters */
static void gattSetAdvertParams(void);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      addDeviceNameToAdvData
 *
 *  DESCRIPTION
 *      This function is used to add a device name to advertisement or scan 
 *      response data. It follows these steps:
 *      a. Try to add the complete device name to the advertisement packet
 *      b. Try to add the complete device name to the scan response packet
 *      c. Try to add the shortened device name to the advertisement packet
 *      d. Try to add the shortened (max possible) device name to the scan 
 *         response packet.
 *
 *  PARAMETERS
 *      adv_data_len [in]       Length of advertisement packet
 *      scan_data_len [in]      Length of scan response packet
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void addDeviceNameToAdvData(uint16 adv_data_len, uint16 scan_data_len)
{

    uint8 *p_device_name = NULL;        /* Device name */
    uint16 device_name_adtype_len;      /* Device name length including AD Type
                                         */

    /* Read device name along with AD Type and its length */
    p_device_name = GapGetNameAndLength(&device_name_adtype_len);

    /* Add complete device name to Advertisement data */
    p_device_name[0] = AD_TYPE_LOCAL_NAME_COMPLETE;

    /* Increment device_name_adtype_len by one to account for length field
     * which will be added by the GAP layer. 
     */

    /* Check if Complete Device Name can fit in remaining advertisement 
     * data space 
     */
    if((device_name_adtype_len + 1) <= (MAX_ADV_DATA_LEN - adv_data_len))
    {
        /* Add Complete Device Name to Advertisement Data */
        if (LsStoreAdvScanData(device_name_adtype_len, p_device_name, 
                      ad_src_advertise) != ls_err_none)
        {
            ReportPanic(app_panic_set_advert_data);
        }

    }
    /* Check if Complete Device Name can fit in Scan response message */
    else if((device_name_adtype_len + 1) <= (MAX_ADV_DATA_LEN - scan_data_len)) 
    {
        /* Add Complete Device Name to Scan Response Data */
        if (LsStoreAdvScanData(device_name_adtype_len, p_device_name, 
                      ad_src_scan_rsp) != ls_err_none)
        {
            ReportPanic(app_panic_set_scan_rsp_data);
        }

    }
    /* Check if Shortened Device Name can fit in remaining advertisement 
     * data space 
     */
    else if((MAX_ADV_DATA_LEN - adv_data_len) >=
            (SHORTENED_DEV_NAME_LEN + 2)) /* Added 2 for Length and AD Type 
                                           * added by GAP layer
                                           */
    {
        /* Add shortened device name to Advertisement data */
        p_device_name[0] = AD_TYPE_LOCAL_NAME_SHORT;

       if (LsStoreAdvScanData(SHORTENED_DEV_NAME_LEN, p_device_name, 
                      ad_src_advertise) != ls_err_none)
        {
            ReportPanic(app_panic_set_advert_data);
        }

    }
    else /* Add device name to remaining Scan response data space */
    {
        /* Add as much as can be stored in Scan Response data */
        p_device_name[0] = AD_TYPE_LOCAL_NAME_SHORT;

       if (LsStoreAdvScanData(MAX_ADV_DATA_LEN - scan_data_len, 
                                    p_device_name, 
                                    ad_src_scan_rsp) != ls_err_none)
        {
            ReportPanic(app_panic_set_scan_rsp_data);
        }

    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      gattSetAdvertParams
 *
 *  DESCRIPTION
 *      This function is used to set advertisement parameters.
 *
 *  PARAMETERS
 *      None; this application doesn't support pairing.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void gattSetAdvertParams(void)
{
    uint8 advert_data[MAX_ADV_DATA_LEN];/* Advertisement packet */
    uint16 length;                      /* Length of advertisement packet */
    /* Advertisement interval, microseconds */
    uint32 adv_interval_min = GS_ADVERTISING_INTERVAL_MIN;
    uint32 adv_interval_max = GS_ADVERTISING_INTERVAL_MAX;
    /* Transmit power level, dBm */
    int8 tx_power_level = -1;

    /* Tx power level value prefixed with 'Tx Power' AD Type */
    /* Refer to BT4.0 specification, Vol3-part-C-Section-11.1.5 */ 
    uint8 device_tx_power[TX_POWER_VALUE_LENGTH] = {
                AD_TYPE_TX_POWER
                };

    /* Device appearance */
    uint8 device_appearance[ATTR_LEN_DEVICE_APPEARANCE + 1] = {
                AD_TYPE_APPEARANCE,
                WORD_LSB(APPEARANCE_APPLICATION_VALUE),
                WORD_MSB(APPEARANCE_APPLICATION_VALUE)
                };

    /* A variable to keep track of the data added to advert_data. The limit is 
     * MAX_ADV_DATA_LEN. GAP layer will add AD Flags to advert_data which is 3
     * bytes. Refer BT Spec 4.0, Vol 3, Part C, Sec 11.1.3:
     *
     * First byte is length
     * second byte is AD TYPE = 0x1
     * Third byte is Flags description 
     */
    uint16 length_added_to_adv = 3;

    if((GapSetMode(gap_role_peripheral, gap_mode_discover_general,
                        gap_mode_connect_undirected, 
                        gap_mode_bond_no,
                        gap_mode_security_none) != ls_err_none) ||
       (GapSetAdvInterval(adv_interval_min, adv_interval_max) 
                        != ls_err_none))
    {
        ReportPanic(app_panic_set_advert_params);
    }


    /* Reset existing advertising data */
    if(LsStoreAdvScanData(0, NULL, ad_src_advertise) != ls_err_none)
    {
        ReportPanic(app_panic_set_advert_data);
    }

    /* Reset existing scan response data */
    if(LsStoreAdvScanData(0, NULL, ad_src_scan_rsp) != ls_err_none)
    {
        ReportPanic(app_panic_set_scan_rsp_data);
    }

    /* Setup ADVERTISEMENT DATA */

    /* Add UUID list of the services supported by the device */
    length = GetSupported128BitUUIDServiceList(advert_data);    

    /* One added for Length field, which will be added to Adv Data by GAP 
     * layer 
     */
    length_added_to_adv += (length + 1);

    if (LsStoreAdvScanData(length, advert_data, 
                        ad_src_advertise) != ls_err_none)
    {
        ReportPanic(app_panic_set_advert_data);
    }
    
    /* Add 16 bit uuids */
    //length = GetSupported16BitUUIDServiceList(advert_data);

    /* One added for Length field, which will be added to Adv Data by GAP 
     * layer 
     */
    //length_added_to_adv += (length + 1);

    //if (LsStoreAdvScanData(length, advert_data, 
    //                    ad_src_advertise) != ls_err_none)
    //{
    //    ReportPanic(app_panic_set_advert_data);
    //}

    /* One added for Length field, which will be added to Adv Data by GAP 
     * layer 
     */
    length_added_to_adv += (sizeof(device_appearance) + 1);

    /* Add device appearance to the advertisements */
    if (LsStoreAdvScanData(ATTR_LEN_DEVICE_APPEARANCE + 1, 
        device_appearance, ad_src_advertise) != ls_err_none)
    {
        ReportPanic(app_panic_set_advert_data);
    }

    /* Read tx power of the chip */
    if(LsReadTransmitPowerLevel(&tx_power_level) != ls_err_none)
    {
        /* Reading tx power failed */
        ReportPanic(app_panic_read_tx_pwr_level);
    }

    /* Add the read tx power level to device_tx_power 
     * Tx power level value is of 1 byte 
     */
    device_tx_power[TX_POWER_VALUE_LENGTH - 1] = (uint8 )tx_power_level;

    /* One added for Length field, which will be added to Adv Data by GAP 
     * layer 
     */
    length_added_to_adv += (TX_POWER_VALUE_LENGTH + 1);

    /* Add tx power value of device to the advertising data */
    if (LsStoreAdvScanData(TX_POWER_VALUE_LENGTH, device_tx_power, 
                          ad_src_advertise) != ls_err_none)
    {
        ReportPanic(app_panic_set_advert_data);
    }

    addDeviceNameToAdvData(length_added_to_adv, 0);

}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      InitGattData
 *
 *  DESCRIPTION
 *      This function initialises the application GATT data.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void InitGattData(void)
{
    g_gatt_data.advert_timer_value = TIMER_INVALID;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles read operations on attributes (as received in 
 *      GATT_ACCESS_IND message) maintained by the application and responds
 *      with the GATT_ACCESS_RSP message.
 *
 *  PARAMETERS
 *      p_ind [in]              Data received in GATT_ACCESS_IND message.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void HandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    /* For the received attribute handle, check all the services that support 
     * attribute 'Read' operation handled by application.
     */
    /* More services may be added here to support their read operations */
    if(GapCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to the GAP service */
        GapHandleAccessRead(p_ind);
    }
    else if(DeviceInfoCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to the DEVICE INFORMATION service */
        DeviceInfoHandleAccessRead(p_ind);
    }
    else if(BatteryCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to BATTERY service */
        BatteryHandleAccessRead(p_ind);
    } else if (AuthCheckHandleRange(p_ind->handle)) {
        /* attribute handle belongs to authentication service */
        AuthHandleAccessRead(p_ind);
    }
    else
    {
        /* Application doesn't support 'Read' operation on received attribute
         * handle, so return 'gatt_status_read_not_permitted' status.
         */
        GattAccessRsp(p_ind->cid, p_ind->handle, 
                      gatt_status_read_not_permitted,
                      0, NULL);
    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleAccessWrite
 *
 *  DESCRIPTION
 *      This function handles write operations on attributes (as received in 
 *      GATT_ACCESS_IND message) maintained by the application and responds
 *      with the GATT_ACCESS_RSP message.
 *
 *  PARAMETERS
 *      p_ind [in]              Data received in GATT_ACCESS_IND message.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void HandleAccessWrite(GATT_ACCESS_IND_T *p_ind)
{
    /* For the received attribute handle, check all the services that support 
     * attribute 'Write' operation handled by application.
     */
    /* More services may be added here to support their write operations */
    if(GapCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to GAP service */
        GapHandleAccessWrite(p_ind);
    }
    else if(BatteryCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to BATTERY service */
        BatteryHandleAccessWrite(p_ind);
    } else if (AuthCheckHandleRange(p_ind->handle)) {
        /* attribute handle belongs to auth service */
        AuthHandleAccessWrite(p_ind);
    }
    else
    {
        /* Application doesn't support 'Write' operation on received  attribute
         * handle, so return 'gatt_status_write_not_permitted' status
         */
        GattAccessRsp(p_ind->cid, p_ind->handle, 
                      gatt_status_write_not_permitted,
                      0, NULL);
    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattStartAdverts
 *
 *  DESCRIPTION
 *      This function is used to start undirected advertisements and moves to 
 *      ADVERTISING state.
 *
 *  PARAMETERS
 *      p_addr [in]             Bonded host address
 *      fast_connection [in]    TRUE:  Fast advertisements
 *                              FALSE: Slow advertisements
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void GattStartAdverts(void)
{
    /* Variable 'connect_flags' needs to be updated to have peer address type 
     * if Directed advertisements are supported as peer address type will 
     * only be used in that case. We don't support directed advertisements for 
     * this application.
     */
#ifdef USE_STATIC_RANDOM_ADDRESS
    uint16 connect_flags = L2CAP_CONNECTION_SLAVE_UNDIRECTED | 
                           L2CAP_OWN_ADDR_TYPE_RANDOM;
#else
    uint16 connect_flags = L2CAP_CONNECTION_SLAVE_UNDIRECTED | 
                           L2CAP_OWN_ADDR_TYPE_PUBLIC;
#endif /* USE_STATIC_RANDOM_ADDRESS */

    /* Set advertisement parameters */
    gattSetAdvertParams();

    /* Start GATT connection in Slave role */
    GattConnectReq(NULL, connect_flags);

     /* Start advertisement timer */
    if(g_gatt_data.advert_timer_value)
    {
        StartAdvertTimer(g_gatt_data.advert_timer_value);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetSupported16BitUUIDServiceList
 *
 *  DESCRIPTION
 *      This function prepares the list of supported 16-bit service UUIDs to be 
 *      added to Advertisement data. It also adds the relevant AD Type to the 
 *      start of AD array.
 *
 *  PARAMETERS
 *      p_service_uuid_ad [out] AD Service UUID list
 *
 *  RETURNS
 *      Size of AD Service UUID list
 *----------------------------------------------------------------------------*/
extern uint16 GetSupported16BitUUIDServiceList(uint8 *p_service_uuid_ad)
{
    uint8   size_data = 0;              /* Size of AD Service UUID list */

    /* Add 16-bit UUID for supported main service */
    p_service_uuid_ad[size_data++] = AD_TYPE_SERVICE_UUID_16BIT_LIST;

    /* Add battery service UUID */
    p_service_uuid_ad[size_data++] = WORD_LSB(UUID_BATTERY_SERVICE);
    p_service_uuid_ad[size_data++] = WORD_MSB(UUID_BATTERY_SERVICE);

    /* Add device info service UUID */
    p_service_uuid_ad[size_data++] = WORD_LSB(UUID_DEVICE_INFO_SERVICE);
    p_service_uuid_ad[size_data++] = WORD_MSB(UUID_DEVICE_INFO_SERVICE);

    /* Add all the supported UUIDs in this function*/

    /* Return the size of AD service data. */
    return ((uint16)size_data);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetSupported128BitUUIDServiceList
 *
 *  DESCRIPTION
 *      This function prepares the list of supported 128-bit service UUIDs to be 
 *      added to Advertisement data. It also adds the relevant AD Type to the 
 *      start of AD array.
 *
 *  PARAMETERS
 *      p_service_uuid_ad [out] AD Service UUID list
 *
 *  RETURNS
 *      Size of AD Service UUID list
 *----------------------------------------------------------------------------*/
extern uint16 GetSupported128BitUUIDServiceList(uint8 *p_service_uuid_ad)
{
    uint8   size_data = 0;              /* Size of AD Service UUID list */

    /* Add 16-bit UUID for supported main service */
    p_service_uuid_ad[size_data++] = AD_TYPE_SERVICE_UUID_128BIT_LIST;

    /* Add authentication service UUID */
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_16;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_15;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_14;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_13;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_12;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_11;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_10;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_9;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_8;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_7;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_6;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_5;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_4;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_3;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_2;
    p_service_uuid_ad[size_data++] = UUID_AUTH_SERVICE_1;

    /* Return the size of AD service data. */
    return ((uint16)size_data);

}
/*----------------------------------------------------------------------------*
 *  NAME
 *      GattIsAddressResolvableRandom
 *
 *  DESCRIPTION
 *      This function checks if the address is resolvable random or not.
 *
 *  PARAMETERS
 *      p_addr [in]             Address to check
 *
 *  RETURNS
 *      TRUE if supplied address is a resolvable private address
 *      FALSE if supplied address is non-resolvable private address
 *----------------------------------------------------------------------------*/
extern bool GattIsAddressResolvableRandom(TYPED_BD_ADDR_T *p_addr)
{
    if(p_addr->type != L2CA_RANDOM_ADDR_TYPE || 
      (p_addr->addr.nap & BD_ADDR_NAP_RANDOM_TYPE_MASK)
                                      != BD_ADDR_NAP_RANDOM_TYPE_RESOLVABLE)
    {
        /* This is not a resolvable private address  */
        return FALSE;
    }
    return TRUE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattTriggerFastAdverts
 *
 *  DESCRIPTION
 *      This function is used to trigger fast advertisements.
 *
 *  PARAMETERS
 *      p_addr [in]             Bonded host address
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void GattTriggerFastAdverts(TYPED_BD_ADDR_T *p_addr)
{
    g_gatt_data.advert_timer_value = FAST_CONNECTION_ADVERT_TIMEOUT_VALUE;

    /* Trigger fast connections */
    GattStartAdverts();
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattStopAdverts
 *
 *  DESCRIPTION
 *      This function is used to stop advertisements.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void GattStopAdverts(void)
{
    switch(GetState())
    {
        case app_state_fast_advertising:
        {
            if(IsWhiteListEnabled())
            {
                /* Set advertisement timer for remaining 20 seconds for fast
                 * connections without any device in the white list.
                 */
                g_gatt_data.advert_timer_value = 
                                FAST_CONNECTION_ADVERT_TIMEOUT_VALUE - 
                                BONDED_DEVICE_ADVERT_TIMEOUT_VALUE;
            }
            else
            {
                /* No advertisement timer for reduced power connections. */
                g_gatt_data.advert_timer_value = 
                            SLOW_CONNECTION_ADVERT_TIMEOUT_VALUE;
            }
        }
        /* FALLTHROUGH*/

        case app_state_slow_advertising:
            /* Stop on-going advertisements */
            GattCancelConnectReq();
        break;

        default:
            /* Ignore timer in remaining states */
        break;
    }
}

/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      gatt_server.c
 *
 *  DESCRIPTION
 *      This file defines a simple implementation of a GATT server.
 *
 ******************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <main.h>           /* Functions relating to powering up the device */
#include <types.h>          /* Commonly used type definitions */
#include <timer.h>          /* Chip timer functions */
#include <mem.h>            /* Memory library */
#include <config_store.h>   /* Interface to the Configuration Store */

/* Upper Stack API */
#include <gatt.h>           /* GATT application interface */
#include <ls_app_if.h>      /* Link Supervisor application interface */
#include <gap_app_if.h>     /* GAP application interface */
#include <buf_utils.h>      /* Buffer functions */
#include <security.h>       /* Security Manager application interface */
#include <panic.h>          /* Support for applications to panic */
#include <nvm.h>            /* Access to Non-Volatile Memory */
#include <random.h>         /* Generators for pseudo-random data sequences */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
                                
#include "gatt_access.h"    /* GATT-related routines */
#include "app_gatt_db.h"    /* GATT database definitions */
#include "nvm_access.h"     /* Non-volatile memory access */
#include "cryptobeacon.h"    /* Definitions used throughout the application */
#include "hw_access.h"      /* Hardware access */
#include "debug_interface.h"/* Application debug routines */
#include "gap_service.h"    /* GAP service interface */
#include "battery_service.h"/* Battery service interface */
#include "auth_service.h"   /* Authentication service interface */
#include "power_management.h"
#include "nvm_map.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Maximum number of timers. Up to four timers are required by this application:
 *  
 *  This file:      con_param_update_tid
 *  This file:      app_tid
 *  hw_access.c:    button_press_tid
 */
#define MAX_APP_TIMERS                 (3)

/* Slave device is not allowed to transmit another Connection Parameter 
 * Update request till time TGAP(conn_param_timeout). Refer to section 9.3.9.2,
 * Vol 3, Part C of the Core 4.0 BT spec. The application should retry the 
 * 'Connection Parameter Update' procedure after time TGAP(conn_param_timeout)
 * which is 30 seconds.
 */
#define GAP_CONN_PARAM_TIMEOUT          (30 * SECOND)

/*============================================================================*
 *  Private Data types
 *============================================================================*/

/* Application data structure */
typedef struct _APP_DATA_T
{
    /* Current state of application */
    app_state                  state;

    /* TYPED_BD_ADDR_T of the host to which device is connected */
    TYPED_BD_ADDR_T            con_bd_addr;

    /* Track the Connection Identifier (UCID) as Clients connect and
     * disconnect
     */
    uint16                     st_ucid;

    /* Timer ID for Connection Parameter Update timer in Connected state */
    timer_id                   con_param_update_tid;

    /* Central Private Address Resolution IRK. Will only be used when
     * central device used resolvable random address.
     */
    uint16                     irk[MAX_WORDS_IRK];

    /* Number of connection parameter update requests made */
    uint8                      num_conn_update_req;

    /* Boolean flag to indicate pairing button press */
    bool                       pairing_button_pressed;

    /* Timer ID for 'UNDIRECTED ADVERTS' and activity on the sensor device like
     * measurements or user intervention in CONNECTED state.
     */
    timer_id                   app_tid;


    /* Current connection interval */
    uint16                     conn_interval;

    /* Current slave latency */
    uint16                     conn_latency;

    /* Current connection timeout value */
    uint16                     conn_timeout;
} APP_DATA_T;

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* Declare space for application timers */
static uint16 app_timers[SIZEOF_APP_TIMER * MAX_APP_TIMERS];

/* Application data instance */
static APP_DATA_T g_app_data;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Initialise application data structure */
static void appDataInit(void);

/* Initialise and read NVM data */
static void readPersistentStore(void);

#if defined(CONNECTED_IDLE_TIMEOUT_VALUE)
    /* Handle Idle timer expiry in connected states */
    static void appIdleTimerHandler(timer_id tid);

    /* Reset the time for which the application was idle */
    static void resetIdleTimer(void);
#endif /* CONNECTED_IDLE_TIMEOUT_VALUE */

/* Start the Connection update timer */
static void appStartConnUpdateTimer(void);

/* Send L2CAP_CONNECTION_PARAMETER_UPDATE_REQUEST to the remote device */
static void requestConnParamUpdate(timer_id tid);

/* Exit the advertising states */
static void appExitAdvertising(void);

/* Handle advertising timer expiry */
static void appAdvertTimerHandler(timer_id tid);

/* LM_EV_CONNECTION_COMPLETE signal handler */
static void handleSignalLmEvConnectionComplete(
                                     LM_EV_CONNECTION_COMPLETE_T *p_event_data);

/* GATT_ADD_DB_CFM signal handler */
static void handleSignalGattAddDbCfm(GATT_ADD_DB_CFM_T *p_event_data);

/* GATT_CANCEL_CONNECT_CFM signal handler */
static void handleSignalGattCancelConnectCfm(void);

/* GATT_CONNECT_CFM signal handler */
static void handleSignalGattConnectCfm(GATT_CONNECT_CFM_T *p_event_data);

/* SM_KEYS_IND signal handler */
static void handleSignalSmKeysInd(SM_KEYS_IND_T *p_event_data);

/* SM_SIMPLE_PAIRING_COMPLETE_IND signal handler */
static void handleSignalSmSimplePairingCompleteInd(
                    SM_SIMPLE_PAIRING_COMPLETE_IND_T *p_event_data);

/* SM_DIV_APPROVE_IND signal handler */
static void handleSignalSmDivApproveInd(SM_DIV_APPROVE_IND_T *p_event_data);

/* LS_CONNECTION_PARAM_UPDATE_CFM signal handler */
static void handleSignalLsConnParamUpdateCfm(
                    LS_CONNECTION_PARAM_UPDATE_CFM_T *p_event_data);

/* LS_CONNECTION_PARAM_UPDATE_IND signal handler */
static void handleSignalLsConnParamUpdateInd(
                    LS_CONNECTION_PARAM_UPDATE_IND_T *p_event_data);

/* GATT_ACCESS_IND signal handler */
static void handleSignalGattAccessInd(GATT_ACCESS_IND_T *p_event_data);

/* LM_EV_DISCONNECT_COMPLETE signal handler */
static void handleSignalLmDisconnectComplete(
                    HCI_EV_DATA_DISCONNECT_COMPLETE_T *p_event_data);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      appDataInit
 *
 *  DESCRIPTION
 *      This function is called to initialise the application data structure.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void appDataInit(void)
{
    /* Initialise general application timer */
    if (g_app_data.app_tid != TIMER_INVALID)
    {
        TimerDelete(g_app_data.app_tid);
        g_app_data.app_tid = TIMER_INVALID;
    }

    /* Reset the pairing button press flag */
    g_app_data.pairing_button_pressed = FALSE;

    /* Initialise the connection parameter update timer */
    if (g_app_data.con_param_update_tid != TIMER_INVALID)
    {
        TimerDelete(g_app_data.con_param_update_tid);
        g_app_data.con_param_update_tid = TIMER_INVALID;
    }

    /* Initialise the connected client ID */
    g_app_data.st_ucid = GATT_INVALID_UCID;

    /* Reset the connection parameter variables */
    g_app_data.conn_interval = 0;
    g_app_data.conn_latency = 0;
    g_app_data.conn_timeout = 0;

    /* Initialise the application GATT data */
    InitGattData();
    
    /* Reset application hardware data */
    HwDataReset();

    /* Initialise GAP data structure */
    GapDataInit();

    /* Battery Service data initialisation */
    BatteryDataInit();
    
    /* Auth Service data initialization */
    InitAuthServiceData();

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      readPersistentStore
 *
 *  DESCRIPTION
 *      This function is used to initialise and read NVM data.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void readPersistentStore(void)
{
    /* NVM offset for supported services */
    uint16 nvm_offset = NVM_MAX_APP_MEMORY_WORDS;
    uint16 nvm_sanity = 0xffff;
    bool nvm_start_fresh = FALSE;

    /* Read persistent storage to find if the device was last bonded to another
     * device. If the device was bonded, trigger fast undirected advertisements
     * by setting the white list for bonded host. If the device was not bonded,
     * trigger undirected advertisements for any host to connect.
     */
    
    Nvm_Read(&nvm_sanity, 
             sizeof(nvm_sanity), 
             NVM_ADDR_SANITY_WORD);

    if(nvm_sanity == NVM_SANITY_MAGIC)
    {

        /* Valid NVM */

        /* If NVM in use, read device name and length from NVM */
        GapReadDataFromNVM(&nvm_offset);

    }
    else /* NVM Sanity check failed means either the device is being brought up 
          * for the first time or memory has got corrupted in which case 
          * discard the data and start fresh.
          */
    {

        nvm_start_fresh = TRUE;

        nvm_sanity = NVM_SANITY_MAGIC;

        /* Write NVM Sanity word to the NVM */
        Nvm_Write(&nvm_sanity, 
                  sizeof(nvm_sanity), 
                  NVM_ADDR_SANITY_WORD);

        /* If fresh NVM, write device name and length to NVM for the 
         * first time.
         */
        GapInitWriteDataToNVM(&nvm_offset);

    }

    /* Read Battery service data from NVM if the devices are bonded and  
     * update the offset with the number of words of NVM required by 
     * this service
     */
    BatteryReadDataFromNVM(&nvm_offset);
    
    /* Add the 'read Service data from NVM' API call here, to initialise the
     * service data, if the device is already bonded. One must take care of the
     * offset being used for storing the data.
     */

}

#if defined(CONNECTED_IDLE_TIMEOUT_VALUE)
/*----------------------------------------------------------------------------*
 *  NAME
 *      appIdleTimerHandler
 *
 *  DESCRIPTION
 *      This function is used to handle Idle timer expiry in connected states.
 *      At the expiry of this timer, application shall disconnect with the 
 *      host and shall move to 'APP_DISCONNECTING' state.
 *
 *  PARAMETERS
 *      tid [in]                ID of timer that has expired
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void appIdleTimerHandler(timer_id tid)
{
    if(tid == g_app_data.app_tid)
    {
        /* Timer has just expired, so mark it as invalid */
        g_app_data.app_tid = TIMER_INVALID;

        /* Handle signal as per current state */
        switch(g_app_data.state)
        {
            case app_state_connected:
            {
                /* Trigger Disconnect and move to app_state_disconnecting 
                 * state 
                 */
                SetState(app_state_disconnecting);
            }
            break;

            default:
                /* Ignore timer in any other state */
            break;
        }

    } /* Else ignore the timer */
}
#endif /* CONNECTED_IDLE_TIMEOUT_VALUE */

/*----------------------------------------------------------------------------*
 *  NAME
 *      appStartConnUpdateTimer
 *
 *  DESCRIPTION
 *      This function starts the connection parameter update timer.
 *      g_app_data.con_param_update_tid must be TIMER_INVALID on entry to this
 *      function.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void appStartConnUpdateTimer(void)
{
    if(g_app_data.conn_interval < PREFERRED_MIN_CON_INTERVAL ||
       g_app_data.conn_interval > PREFERRED_MAX_CON_INTERVAL
#if PREFERRED_SLAVE_LATENCY
       || g_app_data.conn_latency < PREFERRED_SLAVE_LATENCY
#endif
      )
    {
        /* Set the number of connection parameter update attempts to zero */
        g_app_data.num_conn_update_req = 0;

        /* Start timer to trigger connection parameter update procedure */
        g_app_data.con_param_update_tid = TimerCreate(
                            GAP_CONN_PARAM_TIMEOUT,
                            TRUE, requestConnParamUpdate);

    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      requestConnParamUpdate
 *
 *  DESCRIPTION
 *      This function is used to send L2CAP_CONNECTION_PARAMETER_UPDATE_REQUEST 
 *      to the remote device.
 *
 *  PARAMETERS
 *      tid [in]                ID of timer that has expired
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void requestConnParamUpdate(timer_id tid)
{
    /* Application specific preferred parameters */
     ble_con_params app_pref_conn_param = 
                {
                    PREFERRED_MIN_CON_INTERVAL,
                    PREFERRED_MAX_CON_INTERVAL,
                    PREFERRED_SLAVE_LATENCY,
                    PREFERRED_SUPERVISION_TIMEOUT
                };

    if(g_app_data.con_param_update_tid == tid)
    {
        /* Timer has just expired, so mark it as being invalid */
        g_app_data.con_param_update_tid = TIMER_INVALID;

        /* Handle signal as per current state */
        switch(g_app_data.state)
        {

            case app_state_connected:
            {
                /* Send Connection Parameter Update request using application 
                 * specific preferred connection parameters
                 */

                if(LsConnectionParamUpdateReq(&g_app_data.con_bd_addr, 
                                &app_pref_conn_param) != ls_err_none)
                {
                    ReportPanic(app_panic_con_param_update);
                }

                /* Increment the count for connection parameter update 
                 * requests 
                 */
                ++ g_app_data.num_conn_update_req;

            }
            break;

            default:
                /* Ignore in other states */
            break;
        }

    } /* Else ignore the timer */

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      appExitAdvertising
 *
 *  DESCRIPTION
 *      This function is called while exiting app_state_fast_advertising and
 *      app_state_slow_advertising states.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void appExitAdvertising(void)
{
    /* Cancel advertisement timer. Must be valid because timer is active
     * during app_state_fast_advertising and app_state_slow_advertising states.
     */
    TimerDelete(g_app_data.app_tid);
    g_app_data.app_tid = TIMER_INVALID;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      appAdvertTimerHandler
 *
 *  DESCRIPTION
 *      This function is used to handle Advertisement timer expiry.
 *
 *  PARAMETERS
 *      tid [in]                ID of timer that has expired
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void appAdvertTimerHandler(timer_id tid)
{
    /* Based upon the timer id, stop on-going advertisements */
    if(g_app_data.app_tid == tid)
    {
        /* Timer has just expired so mark it as invalid */
        g_app_data.app_tid = TIMER_INVALID;

        GattStopAdverts();
    }/* Else ignore timer expiry, could be because of 
      * some race condition */
}

#if defined(CONNECTED_IDLE_TIMEOUT_VALUE)
/*----------------------------------------------------------------------------*
 *  NAME
 *      resetIdleTimer
 *
 *  DESCRIPTION
 *      This function is used to reset the time for which the application was
 *      idle during the connected state.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void resetIdleTimer(void)
{
    /* Delete the Idle timer, if already running */
    if (g_app_data.app_tid != TIMER_INVALID)
    {
        TimerDelete(g_app_data.app_tid);
    }

    /* Start the Idle timer again.*/
    g_app_data.app_tid  = TimerCreate(CONNECTED_IDLE_TIMEOUT_VALUE, 
                                    TRUE, appIdleTimerHandler);
}
#endif /* CONNECTED_IDLE_TIMEOUT_VALUE */

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattAddDbCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_ADD_DB_CFM.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by GATT_ADD_DB_CFM signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalGattAddDbCfm(GATT_ADD_DB_CFM_T *p_event_data)
{
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_init:
        {
            if(p_event_data->result == sys_status_success)
            {
                /* Start advertising. */
                SetState(app_state_beaconing);
            }
            else
            {
                /* This should never happen */
                ReportPanic(app_panic_db_registration);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLmEvConnectionComplete
 *
 *  DESCRIPTION
 *      This function handles the signal LM_EV_CONNECTION_COMPLETE.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by LM_EV_CONNECTION_COMPLETE
 *                              signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalLmEvConnectionComplete(
                                     LM_EV_CONNECTION_COMPLETE_T *p_event_data)
{
    /* Store the connection parameters. */
    g_app_data.conn_interval = p_event_data->data.conn_interval;
    g_app_data.conn_latency = p_event_data->data.conn_latency;
    g_app_data.conn_timeout = p_event_data->data.supervision_timeout;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattCancelConnectCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_CANCEL_CONNECT_CFM.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalGattCancelConnectCfm(void)
{
    if(g_app_data.pairing_button_pressed)
    {
        /* Pairing removal has been initiated by the user */
        g_app_data.pairing_button_pressed = FALSE;

        /* Trigger fast advertisements */
        if(g_app_data.state == app_state_beaconing)
        {
            GattStartAdverts();
        }
        else
        {
            SetState(app_state_beaconing);
        }
    }
    else
    {
        switch(g_app_data.state)
        {
            case app_state_beaconing:
            {
                /* shouldn't happen, removed advertising timers. Just in
                   case, re-enter beaconing state.                      */
                SetState(app_state_beaconing);
            }
            break;
        
            default:
                /* Control should never come here */
                ReportPanic(app_panic_invalid_state);
            break;
        }

    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattConnectCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_CONNECT_CFM.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by GATT_CONNECT_CFM signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalGattConnectCfm(GATT_CONNECT_CFM_T *p_event_data)
{
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_beaconing:
        {
            if(p_event_data->result == sys_status_success)
            {
                /* Store received UCID */
                g_app_data.st_ucid = p_event_data->cid;

                /* Store connected BD Address */
                g_app_data.con_bd_addr = p_event_data->bd_addr;
                
                /* Enter connected state 
                 * - If the device is not bonded OR
                 * - If the device is bonded and the connected host doesn't 
                 *   support Resolvable Random address OR
                 * - If the device is bonded and connected host supports 
                 *   Resolvable Random address and the address gets resolved
                 *   using the stored IRK key
                 */
                 SetState(app_state_connected);

                /* if the application does not mandate encryption
                 * requirement on its characteristics, the remote master may
                 * or may not encrypt the link. Start a timer here to give
                 * remote master some time to encrypt the link and on expiry
                 * of that timer, send a connection parameter update request
                 * to remote device.
                 */

                /* If the current connection parameters being used don't 
                 * comply with the application's preferred connection 
                 * parameters and the timer is not running, start a timer
                 * to trigger the Connection Parameter Update procedure.
                 */

                if(g_app_data.con_param_update_tid == TIMER_INVALID)
                {
                    appStartConnUpdateTimer();
                } /* Else at the expiry of the timer the connection
                   * parameter update procedure will be triggered
                   */

            }
            else
            {
                /* Connection failure - Trigger fast advertisements */
                if(g_app_data.state != app_state_beaconing)
                {
                    SetState(app_state_beaconing);
                }
                else
                {
                    /* Already in app_state_beaconing state, so just 
                     * trigger advertisements
                     */
                    GattStartAdverts();
                }
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalSmKeysInd
 *
 *  DESCRIPTION
 *      This function handles the signal SM_KEYS_IND and copies the IRK from it.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by SM_KEYS_IND signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalSmKeysInd(SM_KEYS_IND_T *p_event_data)
{
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        /* Shouldn't ever happen... */

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalSmSimplePairingCompleteInd
 *
 *  DESCRIPTION
 *      This function handles the signal SM_SIMPLE_PAIRING_COMPLETE_IND.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by SM_SIMPLE_PAIRING_COMPLETE_IND
 *                              signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalSmSimplePairingCompleteInd(
                                 SM_SIMPLE_PAIRING_COMPLETE_IND_T *p_event_data)
{

    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_connected:
        {
            if(p_event_data->status == sys_status_success)
            {
                /* Chopped out a ton of code here. Shouldn't allow pairing. */

                /* If the devices are bonded then send notification to all 
                 * registered services for the same so that they can store
                 * required data to NVM.
                 */
                BatteryBondingNotify();
                
                /* Add the Service Bonding Notify API here */
            }
            else
            {
                /* Initialise the data of used services as the device is no 
                 * longer bonded to the remote host.
                 */
                GapDataInit();
                BatteryDataInit();
                
                /* Call all the APIs which initailise the required service(s)
                 * supported by the application.
                 */

            }
        }
        break;

        default:
            /* Firmware may send this signal after disconnection. So don't 
             * panic but ignore this signal.
             */
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalSmDivApproveInd
 *
 *  DESCRIPTION
 *      This function handles the signal SM_DIV_APPROVE_IND.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by SM_DIV_APPROVE_IND signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalSmDivApproveInd(SM_DIV_APPROVE_IND_T *p_event_data)
{
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        
        /* Request for approval from application comes only when pairing is not
         * in progress
         */
        case app_state_connected:
        {
            sm_div_verdict approve_div = SM_DIV_REVOKED;
            
            SMDivApproval(p_event_data->cid, approve_div);
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;

    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLsConnParamUpdateCfm
 *
 *  DESCRIPTION
 *      This function handles the signal LS_CONNECTION_PARAM_UPDATE_CFM.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by LS_CONNECTION_PARAM_UPDATE_CFM
 *                              signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalLsConnParamUpdateCfm(
                            LS_CONNECTION_PARAM_UPDATE_CFM_T *p_event_data)
{
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_connected:
        {
            /* Received in response to the L2CAP_CONNECTION_PARAMETER_UPDATE 
             * request sent from the slave after encryption is enabled. If 
             * the request has failed, the device should send the same request
             * again only after Tgap(conn_param_timeout). Refer Bluetooth 4.0
             * spec Vol 3 Part C, Section 9.3.9 and profile spec.
             */
            if ((p_event_data->status != ls_err_none) &&
                    (g_app_data.num_conn_update_req < 
                    MAX_NUM_CONN_PARAM_UPDATE_REQS))
            {
                /* Delete timer if running */
                if (g_app_data.con_param_update_tid != TIMER_INVALID)
                {
                    TimerDelete(g_app_data.con_param_update_tid);
                }

                g_app_data.con_param_update_tid = TimerCreate(
                                             GAP_CONN_PARAM_TIMEOUT,
                                             TRUE, requestConnParamUpdate);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLsConnParamUpdateInd
 *
 *  DESCRIPTION
 *      This function handles the signal LS_CONNECTION_PARAM_UPDATE_IND.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by LS_CONNECTION_PARAM_UPDATE_IND
 *                              signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalLsConnParamUpdateInd(
                                 LS_CONNECTION_PARAM_UPDATE_IND_T *p_event_data)
{

    /* Handle signal as per current state */
    switch(g_app_data.state)
    {

        case app_state_connected:
        {
            /* Delete timer if running */
            if (g_app_data.con_param_update_tid != TIMER_INVALID)
            {
                TimerDelete(g_app_data.con_param_update_tid);
                g_app_data.con_param_update_tid = TIMER_INVALID;
            }

            /* Store the new connection parameters. */
            g_app_data.conn_interval = p_event_data->conn_interval;
            g_app_data.conn_latency = p_event_data->conn_latency;
            g_app_data.conn_timeout = p_event_data->supervision_timeout;
            
            /* Connection parameters have been updated. Check if new parameters 
             * comply with application preferred parameters. If not, application
             * shall trigger Connection parameter update procedure.
             */
            appStartConnUpdateTimer();
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattAccessInd
 *
 *  DESCRIPTION
 *      This function handles GATT_ACCESS_IND messages for attributes maintained
 *      by the application.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by GATT_ACCESS_IND message
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalGattAccessInd(GATT_ACCESS_IND_T *p_event_data)
{

    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_connected:
        {
            /* Received GATT ACCESS IND with write access */
            if(p_event_data->flags == 
                (ATT_ACCESS_WRITE | 
                 ATT_ACCESS_PERMISSION | 
                 ATT_ACCESS_WRITE_COMPLETE))
            {
                HandleAccessWrite(p_event_data);
            }
            /* Received GATT ACCESS IND with read access */
            else if(p_event_data->flags == 
                (ATT_ACCESS_READ | 
                ATT_ACCESS_PERMISSION))
            {
                HandleAccessRead(p_event_data);
            }
            else
            {
                /* No other request is supported */
                GattAccessRsp(p_event_data->cid, p_event_data->handle, 
                              gatt_status_request_not_supported,
                              0, NULL);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLmDisconnectComplete
 *
 *  DESCRIPTION
 *      This function handles LM Disconnect Complete event which is received
 *      at the completion of disconnect procedure triggered either by the 
 *      device or remote host or because of link loss.
 *
 *  PARAMETERS
 *      p_event_data [in]       Data supplied by LM_EV_DISCONNECT_COMPLETE
 *                              signal
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleSignalLmDisconnectComplete(
                HCI_EV_DATA_DISCONNECT_COMPLETE_T *p_event_data)
{

    /* Set UCID to INVALID_UCID */
    g_app_data.st_ucid = GATT_INVALID_UCID;

    /* Reset the connection parameter variables. */
    g_app_data.conn_interval = 0;
    g_app_data.conn_latency = 0;
    g_app_data.conn_timeout = 0;
    
    /* LM_EV_DISCONNECT_COMPLETE event can have following disconnect reasons:
     *
     * HCI_ERROR_CONN_TIMEOUT - Link Loss case
     * HCI_ERROR_CONN_TERM_LOCAL_HOST - Disconnect triggered by device
     * HCI_ERROR_OETC_* - Other end (i.e., remote host) terminated connection
     */
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_connected:
            /* Initialise Application data instance */
            appDataInit();

            /* FALLTHROUGH */

        case app_state_disconnecting:
        {

            /* Link Loss Case */
            if(p_event_data->reason == HCI_ERROR_CONN_TIMEOUT)
            {
                /* Start undirected advertisements by moving to 
                 * app_state_beaconing state
                 */
                SetState(app_state_beaconing);
            }
            else if(p_event_data->reason == HCI_ERROR_CONN_TERM_LOCAL_HOST)
            {
                /* Removed pairing/bonding code */
                SetState(app_state_beaconing);
            }
            else /* Remote user terminated connection case */
            {
                /* If the device has not bonded but disconnected, it may just 
                 * have discovered the services supported by the application or 
                 * read some un-protected characteristic value like device name 
                 * and disconnected. The application should be connectable 
                 * because the same remote device may want to reconnect and 
                 * bond. If not the application should be discoverable by other 
                 * devices.
                 */
                /* Again, much complexity removed */
                SetState(app_state_beaconing);
            }
        }
        break;
        
        case app_state_dead:
            /* Just to prevent panic. */
            break;
            
        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      ReportPanic
 *
 *  DESCRIPTION
 *      This function calls firmware panic routine and gives a single point 
 *      of debugging any application level panics.
 *
 *  PARAMETERS
 *      panic_code [in]         Code to supply to firmware Panic function.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void ReportPanic(app_panic_code panic_code)
{
    /* Raise panic */
    Panic(panic_code);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleShortButtonPress
 *
 *  DESCRIPTION
 *      This function contains handling of short button press. If connected,
 *      the device disconnects from the connected host else it triggers
 *      advertisements.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void HandleShortButtonPress(void)
{

    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_state_connected:
            /* Disconnect from the connected host */
            SetState(app_state_disconnecting);
            
            /* As per the specification Vendor may choose to initiate the 
             * idle timer which will eventually initiate the disconnect.
             */
             
        break;

        default:
            /* Ignore in remaining states */
        break;

    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      SetState
 *
 *  DESCRIPTION
 *      This function is used to set the state of the application.
 *
 *  PARAMETERS
 *      new_state [in]          State to move to
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void SetState(app_state new_state)
{
    /* Check that the new state is not the same as the current state */
    app_state old_state = g_app_data.state;
    
    if (old_state != new_state)
    {
        /* Exit current state */
        switch (old_state)
        {
            case app_state_init:
                /* Do nothing */
                break;
                
            case app_state_disconnecting:
                /* Common things to do whenever application exits
                 * app_state_disconnecting state.
                 */

                /* Initialise application and used services data structure 
                 * while exiting Disconnecting state
                 */
                appDataInit();
            break;

            case app_state_beaconing:
                /* Common things to do whenever application exits
                 * APP_*_ADVERTISING state.
                 */
                appExitAdvertising();
            break;

            case app_state_connected:
                /* The application may need to maintain the values of some
                 * profile specific data across connections and power cycles.
                 * These values would have changed in 'connected' state. So,
                 * update the values of this data stored in the NVM.
                 */
            break;

            case app_state_dead:
                /* Eventually wake up code goes here */
            break;

            default:
                /* Nothing to do */
            break;
        }

        /* Set new state */
        g_app_data.state = new_state;

        /* Enter new state */
        switch (new_state)
        {
            case app_state_beaconing:
            {
                /* Trigger fast advertisements. */
                GattStartAdverts();

                /* Indicate advertising mode on LED. */
                IndicateAdvertisingServer();
            }
            break;

            case app_state_connected:
            {
                /* Common things to do whenever application enters
                 * app_state_connected state.
                 */
                IndicateConnection();

                /* Update battery status at every connection instance. It may 
                 * not be worth updating timer this often, but this will 
                 * depend upon application requirements 
                 */
                BatteryUpdateLevel(g_app_data.st_ucid);

#if defined(CONNECTED_IDLE_TIMEOUT_VALUE)
                resetIdleTimer();
#endif
             }
            break;

            case app_state_disconnecting:
                /* Disconnect the link */
                IndicateUnhandledState();
                GattDisconnectReq(g_app_data.st_ucid);
            break;

            case app_state_dead:
                TimerDelete(g_app_data.app_tid);
                g_app_data.app_tid = TIMER_INVALID;
                GoToSleep();
            break;
            default:
                IndicateUnhandledState();
            break;
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetState
 *
 *  DESCRIPTION
 *      This function returns the current state of the application.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Current application state
 *----------------------------------------------------------------------------*/
extern app_state GetState(void)
{
    return g_app_data.state;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      StartAdvertTimer
 *
 *  DESCRIPTION
 *      This function starts the advertisement timer.
 *
 *  PARAMETERS
 *      interval [in]           Timer duration, microseconds
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void StartAdvertTimer(uint32 interval)
{
    /* Cancel existing timer, if valid */
    if (g_app_data.app_tid != TIMER_INVALID)
    {
        TimerDelete(g_app_data.app_tid);
    }

    /* Start advertisement timer  */
    g_app_data.app_tid = TimerCreate(interval, TRUE, appAdvertTimerHandler);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetConnectionID
 *
 *  DESCRIPTION
 *      This function returns the connection identifier.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Connection identifier.
 *----------------------------------------------------------------------------*/
extern uint16 GetConnectionID(void)
{
    return g_app_data.st_ucid;
}

/*============================================================================*
 *  System Callback Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppPowerOnReset
 *
 *  DESCRIPTION
 *      This user application function is called just after a power-on reset
 *      (including after a firmware panic), or after a wakeup from Hibernate or
 *      Dormant sleep states.
 *
 *      At the time this function is called, the last sleep state is not yet
 *      known.
 *
 *      NOTE: this function should only contain code to be executed after a
 *      power-on reset or panic. Code that should also be executed after an
 *      HCI_RESET should instead be placed in the AppInit() function.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void AppPowerOnReset(void)
{
    /* Code that is only executed after a power-on reset or firmware panic
     * should be implemented here - e.g. configuring application constants
     */
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppInit
 *
 *  DESCRIPTION
 *      This user application function is called after a power-on reset
 *      (including after a firmware panic), after a wakeup from Hibernate or
 *      Dormant sleep states, or after an HCI Reset has been requested.
 *
 *      NOTE: In the case of a power-on reset, this function is called
 *      after AppPowerOnReset().
 *
 *  PARAMETERS
 *      last_sleep_state [in]   Last sleep state
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void AppInit(sleep_state last_sleep_state)
{
    uint16 gatt_db_length = 0;  /* GATT database size */
    uint16 *p_gatt_db = NULL;   /* GATT database */
    
    /* Initialise application debug */
    DebugIfInit();
    
    /* Start the debug messages on the UART */
    DebugIfWriteString("\r\n\r\n**************************\r\n");
    DebugIfWriteString("GATT server GAP peripheral\r\n\r\n");

#if defined(USE_STATIC_RANDOM_ADDRESS) && !defined(PAIRING_SUPPORT)
    /* Use static random address for the application */
    GapSetStaticAddress();
#endif

    /* Initialise the GATT Server application state */
    g_app_data.state = app_state_init;

    /* Initialise the application timers */
    TimerInit(MAX_APP_TIMERS, (void*)app_timers);
    
    /* Initialise local timers */
    g_app_data.con_param_update_tid = TIMER_INVALID;
    g_app_data.app_tid = TIMER_INVALID;

    /* Initialise GATT entity */
    GattInit();

    /* Initialise GATT Server H/W */
    InitHardware();

    /* Install GATT Server support for the optional Write procedure.
     * This is mandatory only if the control point characteristic is supported. 
     */
    GattInstallServerWrite();

    /* Don't wakeup on UART RX line */
    SleepWakeOnUartRX(FALSE);

#ifdef NVM_TYPE_EEPROM
    /* Configure the NVM manager to use I2C EEPROM for NVM store */
    NvmConfigureI2cEeprom();
#elif NVM_TYPE_FLASH
    /* Configure the NVM Manager to use SPI flash for NVM store. */
    NvmConfigureSpiFlash();
#endif /* NVM_TYPE_EEPROM */

    Nvm_Disable();

    /* Battery initialisation on chip reset */
    BatteryInitChipReset();

    /* Initialize the GAP data. Needs to be done before readPersistentStore */
    GapDataInit();

    /* Read persistent storage */
    readPersistentStore();

    /* Tell Security Manager module what value it needs to initialise its
     * diversifier to.
     */
    //SMInit(g_app_data.diversifier);
    
    /* Initialise hardware data */
    HwDataInit();

    /* Initialise application data structure */
    appDataInit();

    /* Tell GATT about our database. We will get a GATT_ADD_DB_CFM event when
     * this has completed.
     */
    p_gatt_db = GattGetDatabase(&gatt_db_length);

    GattAddDatabaseReq(gatt_db_length, p_gatt_db);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcesSystemEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a system event, such
 *      as a battery low notification, is received by the system.
 *
 *  PARAMETERS
 *      id   [in]               System event ID
 *      data [in]               Event data
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
void AppProcessSystemEvent(sys_event_id id, void *data)
{
    switch(id)
    {
        case sys_event_battery_low:
        {
            /* Battery low event received - notify the connected host. If 
             * not connected, the battery level will get notified when 
             * device gets connected again
             */
            if(g_app_data.state == app_state_connected)
            {
                BatteryUpdateLevel(g_app_data.st_ucid);
            }
            SetState(app_state_dead);
        }
        break;

        case sys_event_pio_changed:
        {
             /* Handle the PIO changed event. */
             HandlePIOChangedEvent((pio_changed_data*)data);
        }
        break;
            
        default:
            /* Ignore anything else */
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcessLmEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a LM-specific event
 *      is received by the system.
 *
 *  PARAMETERS
 *      event_code [in]         LM event ID
 *      event_data [in]         LM event data
 *
 *  RETURNS
 *      TRUE if the app has finished with the event data; the control layer
 *      will free the buffer.
 *----------------------------------------------------------------------------*/
bool AppProcessLmEvent(lm_event_code event_code, LM_EVENT_T *p_event_data)
{
    switch (event_code)
    {

        /* Handle events received from Firmware */

        case GATT_ADD_DB_CFM:
            /* Attribute database registration confirmation */
            handleSignalGattAddDbCfm((GATT_ADD_DB_CFM_T*)p_event_data);
        break;

        case LM_EV_CONNECTION_COMPLETE:
            /* Handle the LM connection complete event. */
            handleSignalLmEvConnectionComplete(
                                (LM_EV_CONNECTION_COMPLETE_T*)p_event_data);
        break;

        case GATT_CANCEL_CONNECT_CFM:
            /* Confirmation for the completion of GattCancelConnectReq()
             * procedure 
             */
            handleSignalGattCancelConnectCfm();
        break;

        case GATT_CONNECT_CFM:
            /* Confirmation for the completion of GattConnectReq() 
             * procedure
             */
            handleSignalGattConnectCfm((GATT_CONNECT_CFM_T*)p_event_data);
        break;

        case SM_KEYS_IND:
            /* Indication for the keys and associated security information
             * on a connection that has completed Short Term Key Generation 
             * or Transport Specific Key Distribution
             */
            handleSignalSmKeysInd((SM_KEYS_IND_T *)p_event_data);
        break;

        case SM_SIMPLE_PAIRING_COMPLETE_IND:
            /* Indication for completion of Pairing procedure */
            handleSignalSmSimplePairingCompleteInd(
                (SM_SIMPLE_PAIRING_COMPLETE_IND_T *)p_event_data);
        break;

        case LM_EV_ENCRYPTION_CHANGE:
            /* Indication for encryption change event */
#ifdef PAIRING_SUPPORT
            handleSignalLMEncryptionChange(
            (HCI_EV_DATA_ENCRYPTION_CHANGE_T *)&p_event_data->enc_change.data);
#else /* !PAIRING_SUPPORT */
            /* Nothing to do */
#endif /* PAIRING_SUPPORT */

        break;


         case SM_DIV_APPROVE_IND:
            /* Indication for SM Diversifier approval requested by F/W when 
             * the last bonded host exchange keys. Application may or may not
             * approve the diversifier depending upon whether the application 
             * is still bonded to the same host
             */
            handleSignalSmDivApproveInd((SM_DIV_APPROVE_IND_T *)p_event_data);
        break;


        /* Received in response to the LsConnectionParamUpdateReq() 
         * request sent from the slave after encryption is enabled. If 
         * the request has failed, the device should send the same 
         * request again only after Tgap(conn_param_timeout). Refer Bluetooth 4.0 
         * spec Vol 3 Part C, Section 9.3.9 and HID over GATT profile spec 
         * section 5.1.2.
         */
        case LS_CONNECTION_PARAM_UPDATE_CFM:
            handleSignalLsConnParamUpdateCfm(
                            (LS_CONNECTION_PARAM_UPDATE_CFM_T*)p_event_data);
        break;

        case LS_CONNECTION_PARAM_UPDATE_IND:
            /* Indicates completion of remotely triggered Connection 
             * Parameter Update procedure
             */
            handleSignalLsConnParamUpdateInd(
                            (LS_CONNECTION_PARAM_UPDATE_IND_T *)p_event_data);
        break;

        case GATT_ACCESS_IND:
            /* Indicates that an attribute controlled directly by the
             * application (ATT_ATTR_IRQ attribute flag is set) is being 
             * read from or written to.
             */
            handleSignalGattAccessInd((GATT_ACCESS_IND_T *)p_event_data);
        break;

        case GATT_DISCONNECT_IND:
            /* Disconnect procedure triggered by remote host or due to 
             * link loss is considered complete on reception of 
             * LM_EV_DISCONNECT_COMPLETE event. So, it gets handled on 
             * reception of LM_EV_DISCONNECT_COMPLETE event.
             */
        break;

        case GATT_DISCONNECT_CFM:
            /* Confirmation for the completion of GattDisconnectReq()
             * procedure is ignored as the procedure is considered complete 
             * on reception of LM_EV_DISCONNECT_COMPLETE event. So, it gets 
             * handled on reception of LM_EV_DISCONNECT_COMPLETE event.
             */
        break;

        case LM_EV_DISCONNECT_COMPLETE:
        {
            /* Disconnect procedures either triggered by application or remote
             * host or link loss case are considered completed on reception 
             * of LM_EV_DISCONNECT_COMPLETE event
             */
             handleSignalLmDisconnectComplete(
                    &((LM_EV_DISCONNECT_COMPLETE_T *)p_event_data)->data);
        }
        break;

        default:
            /* Ignore any other event */ 
        break;

    }

    return TRUE;
}






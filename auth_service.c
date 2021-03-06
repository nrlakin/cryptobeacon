/*****************************************************************************
 *
 *  FILE
 *      auth_service.c
 *
 *  DESCRIPTION
 *      This file defines routines for using the Authentication service.
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header files
 *============================================================================*/

#include <types.h>
#include <gatt.h>           /* GATT application interface */
#include <bluetooth.h>      /* Bluetooth specific type definitions */
#include <mem.h>
#include <buf_utils.h>

/*============================================================================*
 *  Local Header files
 *============================================================================*/

#include "app_gatt_db.h"    /* GATT database definitions */
#include "auth_service.h"   /* Interface to this file */
#include "crypto.h"         /* Token signing routines */
#include "gatt_access.h"

/*============================================================================*
 *  Local constants
 *============================================================================*/
#define AUTH_TOKEN_LENGTH   (16)

uint16 PrivateKey[8] = {0xFF00, 0xDDEE, 0xBBCC, 0x99AA, 0x7788, 0x5566, 0x3344, 0x1122};

/*============================================================================*
 *  Local function prototypes
 *============================================================================*/
static void signToken(void);
static void notifySigned(uint16 ucid);

typedef struct {
    uint8 input_token[AUTH_TOKEN_LENGTH];
    uint8 signed_token[AUTH_TOKEN_LENGTH];
} AUTH_TOKEN_T;

uint16 signed_token_client_config;
AUTH_TOKEN_T auth_token;

static void signToken(void) {
    //uint16 i;
    //for (i=0; i<AUTH_TOKEN_LENGTH; i++) {
    //    auth_token.signed_token[i] = auth_token.input_token[i];
    //}
    //auth_token.signed_token[i-1]++;
    //auth_token.signed_token[i-1]&=0x00FF;   // deal w/ XAP's weird rollover.
    
    SignTokenAES(auth_token.input_token, auth_token.signed_token, PrivateKey);
}

static void notifySigned(uint16 ucid) {
    if (signed_token_client_config != gatt_client_config_notification) return;
    
    GattCharValueNotification(ucid, HANDLE_SIGNED_TOKEN, 
                                AUTH_TOKEN_LENGTH, auth_token.signed_token);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      InitAuthServiceData
 *
 *  DESCRIPTION
 *      Initialize data for authentication service.
 *
 *  PARAMETERS
 *      None.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void InitAuthServiceData(void) {
    uint16 i;
    
    for(i=0; i<AUTH_TOKEN_LENGTH; i++) {
        auth_token.input_token[i] = 0;
        auth_token.signed_token[i] = 0;
    }
    signed_token_client_config = gatt_client_config_none;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AuthHandleAccessWrite
 *
 *  DESCRIPTION
 *      This function handles write operations on Auth Service attributes
 *      maintained by the application and responds with the GATT_ACCESS_RSP
 *      message.
 *
 *  PARAMETERS
 *      p_ind [in]              Data received in GATT_ACCESS_IND message.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void AuthHandleAccessWrite(GATT_ACCESS_IND_T *p_ind)
{
    uint8 *p_value = p_ind->value;      /* New attribute value */
    uint16 client_config;
    sys_status rc = sys_status_success; /* Function status */
    uint16 length;

    switch(p_ind->handle) {
        case HANDLE_INPUT_TOKEN:
            length = AUTH_TOKEN_LENGTH > p_ind->size_value ? p_ind->size_value : AUTH_TOKEN_LENGTH;
            MemCopy(auth_token.input_token, p_value, length);
            signToken();
            break;
 
        case HANDLE_SIGNED_TOKEN_C_CFG:
            client_config = BufReadUint16(&p_value);
            if ((client_config == gatt_client_config_notification) ||
                    (client_config == gatt_client_config_none)) {
                signed_token_client_config = client_config;
            }
            break;
            
        default:
            rc = gatt_status_write_not_permitted;
        break;

    }

    /* Send ACCESS RESPONSE */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, 0, NULL);
    
    if (p_ind->handle == HANDLE_INPUT_TOKEN) {
        notifySigned(p_ind->cid);
    }

}

extern void AuthHandleAccessRead(GATT_ACCESS_IND_T *p_ind) {
    uint16 length = 0;
    uint8 value[16];
    sys_status rc = sys_status_success;

    switch (p_ind->handle) {
        case HANDLE_SIGNED_TOKEN:
            length = AUTH_TOKEN_LENGTH;
            MemCopy(value, auth_token.signed_token, length);
            break;
            
        default:
            rc = gatt_status_read_not_permitted;
            break;        
    }
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, length, value);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AuthCheckHandleRange
 *
 *  DESCRIPTION
 *      This function is used to check if the handle belongs to the Authentication 
 *      Service.
 *
 *  PARAMETERS
 *      handle [in]             Handle to check
 *
 *  RETURNS
 *      TRUE if handle belongs to the Authentication Service, FALSE otherwise
 *----------------------------------------------------------------------------*/
extern bool AuthCheckHandleRange(uint16 handle)
{
    return ((handle >= HANDLE_AUTH_SERVICE) &&
            (handle <= HANDLE_AUTH_SERVICE_END))
            ? TRUE : FALSE;
}


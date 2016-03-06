/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      nvm_access.c
 *
 *  DESCRIPTION
 *      This file defines routines used by application to access NVM.
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <pio.h>            /* PIO configuration and control functions */
#include <nvm.h>            /* Access to Non-Volatile Memory */
#include <i2c.h>            /* Access to I2C bus */
#include <panic.h>          /* Support for applications to panic */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "nvm_access.h"     /* Interface to this file */
#include "gatt_server.h"    /* Definitions used throughout the GATT server */

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      Nvm_Disable
 *
 *  DESCRIPTION
 *      This function is used to perform the actions necessary to save power on
 *      NVM once read/write operations are done.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
void Nvm_Disable(void)
{
    /* Disable the NVM. */
    NvmDisable();

    /* Pull down the I2C lines to save power. */
    PioSetI2CPullMode(pio_i2c_pull_mode_strong_pull_down);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      Nvm_Read
 *
 *  DESCRIPTION
 *      Read words from the NVM Store after preparing the NVM to be readable. 
 *      After the read operation, perform the actions necessary to save power
 *      on NVM.
 *
 *      Read words starting at the word offset, and store them in the supplied
 *      buffer.
 *
 *  PARAMETERS
 *      buffer [out]            Data read from NVM
 *      length [in]             Number of words of data to read
 *      offset [in]             Offset from which to start reading, in words
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
void Nvm_Read(uint16 *buffer, uint16 length, uint16 offset)
{
    sys_status result;

    /* Read from NVM. Firmware re-enables the NVM if it is disabled */
    result = NvmRead(buffer, length, offset);

    /* Disable NVM to save power after read operation */
    Nvm_Disable();

    /* Report panic if NVM read is not successful */
    if(sys_status_success != result)
    {
        ReportPanic(app_panic_nvm_read);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      Nvm_Write
 *
 *  DESCRIPTION
 *      Write words to the NVM Store after preparing the NVM to be writable. 
 *      After the write operation, perform the actions necessary to save power
 *      on NVM.
 *
 *      Write words from the supplied buffer into the NVM Store, starting at the
 *      word offset.
 *
 *  PARAMETERS
 *      buffer [in]             Data to write to NVM
 *      length [in]             Number of words of data to write
 *      offset [in]             Offset from which to start writing, in words
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
void Nvm_Write(uint16 *buffer, uint16 length, uint16 offset)
{
    sys_status result;          /* Function status */

    /* Write to NVM. Firmware re-enables the NVM if it is disabled */
    result = NvmWrite(buffer, length, offset);

    /* Disable NVM to save power after write operation */
    Nvm_Disable();

    /* Report panic if NVM write is not successful */
    if(sys_status_success != result)
    {
        ReportPanic(app_panic_nvm_write);
    }
}

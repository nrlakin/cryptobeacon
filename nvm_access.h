/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      nvm_access.h
 *
 *  DESCRIPTION
 *      Header definitions for NVM usage.
 *
 ******************************************************************************/

#ifndef __NVM_ACCESS_H__
#define __NVM_ACCESS_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>          /* Commonly used type definitions */

/*============================================================================*
 *  Public Function Prototypes
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
extern void Nvm_Disable(void);

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
extern void Nvm_Read(uint16 *buffer, uint16 length, uint16 offset);

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
extern void Nvm_Write(uint16 *buffer, uint16 length, uint16 offset);

#endif /* __NVM_ACCESS_H__ */

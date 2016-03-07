#ifndef __AUTH_SERVICE_H__
#define __AUTH_SERVICE_H__

#include <types.h>
#include <gatt.h>

extern void InitAuthServiceData(void);
extern void AuthHandleAccessRead(GATT_ACCESS_IND_T *p_ind);
extern void AuthHandleAccessWrite(GATT_ACCESS_IND_T *p_ind);
extern bool AuthCheckHandleRange(uint16 handle);

#endif
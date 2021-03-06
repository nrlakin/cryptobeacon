
#include <types.h>
#include <crypt.h>

#include "crypto.h"

extern void SignTokenAES(uint8 *token, uint8 *destBuff, uint16 *key) {
    uint16 result[8];
    uint16 i;
    
    /* Copy token into proper format for SMEncryptRawAES() */
    for (i=0; i<8; i++) {
        result[7-i] = token[2*i] << 8;
        result[7-i] &= 0xFF00;
        result[7-i] |= (token[2*i + 1])&0x00FF;
    }
    
    SMEncryptRawAes(key, result);
    
    for (i=0; i<8; i++) {
        destBuff[2*i] = (result[7-i]>>8)&0x00FF;
        destBuff[2*i+1] = result[7-i]&0x00FF;
    }
}
 
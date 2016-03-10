#include <types.h>
#include "string_utils.h"

extern uint8 *int16ToString(uint16 value, uint8 *dest) {
    uint8 *ptr = dest;
    uint16 i;
    uint8 nibble;
    
    for (i=0; i<4; i++) {
        nibble = (value >> (4*(3-i))) & 0x000F;
        if (nibble < 10) {
            *ptr++ = nibble + '0';
        } else {
            *ptr++ = nibble - 10 + 'A';
        }
    }
    return ptr;
}
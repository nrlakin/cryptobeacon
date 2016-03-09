#ifndef __NVM_MAP_H__
#define __NVM_MAP_H__

/* Magic value to check the sanity of Non-Volatile Memory (NVM) region used by
 * the application. This value is unique for each application.
 */
#define NVM_SANITY_MAGIC               (0xABAA)

/* NVM address for NVM sanity word */
#define NVM_ADDR_SANITY_WORD         (0x00)

/* Max size of NVM memory */
#define NVM_MAX_APP_MEMORY_WORDS       (1)

#endif
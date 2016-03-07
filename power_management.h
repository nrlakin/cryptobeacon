
#ifndef __POWER_MANAGEMENT_H__
#define __POWER_MANAGEMENT_H__

typedef enum {
    ASLEEP_CRITICAL,
    ASLEEP,
    AWAKE
} power_state_t;

extern void GoToSleep(void);
extern void WakeUp(void);

#endif

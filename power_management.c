#include <types.h>
#include <sleep.h>
#include <reset.h>
#include <gap_app_if.h>

#include "user_config.h"
#include "power_management.h"
#include "hw_access.h"
#include "leds.h"

extern void GoToSleep(void) {
    //uint16 longtime[3] = {0xFFFF, 0xFFFF, 0x3FFF};
    //CsrMeshStop(TRUE);
    //LsStartStopAdvertise(FALSE, whitelist_disabled, ls_addr_type_null);
    //LsStartStopScan(FALSE, whitelist_disabled, ls_addr_type_null);
    ReadyHWForSleep();
    SleepModeChange(sleep_mode_deep);
    //SleepRequest(sleep_state_hibernate, TRUE, longtime);
    //LightHardwarePowerControl(FALSE);
}

extern void WakeUp(void) {
    initPWMs();
    WarmReset();                    // just reset chip on wakeup for now.
    //CsrMeshStart();
    //LightHardwarePowerControl(TRUE);
}

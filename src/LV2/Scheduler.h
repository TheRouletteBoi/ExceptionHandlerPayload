
#ifndef LV2_SCHEDULER_H
#define LV2_SCHEDULER_H

#include "../PPC.h"

namespace lv2 {

class pu_scheduler 
{
public:
    NO_INLINE static void pu_sched_stop_schedule()
    {
        __mtmsrd(2, 1);
    }

    NO_INLINE static void pu_sched_cont_schedule()
    {
        __asm__ volatile(
            "li %%r3, 0;"
            "ori %%r3, %%r3, 0x8002;"
            "lwsync;"
            "mtmsrd %%r3, 1;"
            ::: "%r3"
        );
    }
};


}

#endif // !LV2_SCHEDULER_H
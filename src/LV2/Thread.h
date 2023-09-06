#include "../Types.h"

namespace lv2 {

class process;
class pu_scheduler;

class pu_thread 
{
public:
    char     _0x00[ 0x80 ];
    process* myProcess;
};

class hw_thread 
{
public:
    char          _0x00[ 0x48 ];
    pu_scheduler* scheduler;
    char          _0x50[ 0x68 - 0x50 ];
    pu_thread**   threads;
    char          _0x70[ 0xB0 - 0x70 ];
    process*      unknown;
    char          _0xB8[ 0xC0 - 0xB8 ];
    uint16_t      someStatus;
};

static inline process* ht_current_process()
{
    const auto hardwareThread = (hw_thread*)__mfsprg0();

    if (hardwareThread->someStatus == 3)
        return hardwareThread->unknown;

    const auto puThread = hardwareThread->threads[0];

    if (puThread != NULL)
        return puThread->myProcess;

    return NULL;
}

static inline process* get_current_process()
{
    lv2::pu_scheduler::pu_sched_stop_schedule();

    auto process = lv2::ht_current_process();

    lv2::pu_scheduler::pu_sched_cont_schedule();

    return process;
}

static inline void sys_timer_usleep(usecond_t microseconds)
{
    SYSCALL_FN(&sys_timer_usleep, SYS_TIMER_USLEEP)(microseconds);
}

static inline void ppu_thread_msg_interrupt_exception(uint64_t thread_obj, uint64_t tb_value)
{
    STATIC_FN(&ppu_thread_msg_interrupt_exception, g_LibLV2.ppuThreadMsgInterruptException_opd)(thread_obj, tb_value);
}


}
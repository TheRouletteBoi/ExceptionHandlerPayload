#include "SystemCalls.h"

namespace lv2 {

static inline system_time_t sys_time_get_system_time()
{
    return SYSCALL_FN(&sys_time_get_system_time, SYS_TIME_GET_SYSTEM_TIME)();
}


}
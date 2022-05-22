#include "Hooks.h"
#include "SpinLock.h"
#include "PPC.h"
#include "Libc.h"

#include "LV2/LV2.h"
#include "LV2/Process.h"
#include "LV2/Memory.h"
#include "LV2/Network.h"
#include "LV2/Scheduler.h"
#include "LV2/Thread.h"

#include "Exploit.h"
#include "UserMemory.h"

#include "LV2/PRX.h"

#include "Hex.h"

volatile int SystemCallLock;

extern "C" void SystemCallHookPrepareDispatch();
void PpuThreadMsgInterruptExceptionHook();

void InstallHooks()
{
    SpinLockInit(&SystemCallLock);
    PPCWriteBranchRelative(g_LibLV2.systemCallDispatchBranch, ((OPD_t*)SystemCallHookPrepareDispatch)->Function, true);
}

extern "C" void SystemCallHookProcess(SystemCallContext* callContext)
{

}

void PpuThreadMsgInterruptExceptionHook()
{
    DEBUG_PRINT("PpuThreadMsgInterruptExceptionHook\n");
}
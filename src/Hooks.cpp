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
#include "LV2/PRX.h"
#include "UserMemory.h"
#include "Hex.h"

volatile int SystemCallLock;

extern "C" void SystemCallHookPrepareDispatch();
//extern "C" void PpuThreadMsgInterruptExceptionHookPrepare();

extern "C" void SystemCallHookProcess(SystemCallContext* callContext)
{
    /*if (callContext->index == _SYS_PRX_LOAD_MODULE)
    {
        lv2::process* process  lv2::get_current_process();
        if (process != nullptr)
        {
            if (process->ImageName != nullptr)
            {
                if (NonCryptoHashFNV1A64(process->ImageName) == NonCryptoHashFNV1A64("/app_home/PS3_GAME/USRDIR/EBOOT.BIN")
                    || NonCryptoHashFNV1A64(process->ImageName) == NonCryptoHashFNV1A64("/dev_bdvd/PS3_GAME/USRDIR/EBOOT.BIN")
                    || strstr(process->ImageName, "PS3_GAME/USRDIR/")
                    || strstr(process->ImageName, "hdd0/game/"))
                {
                    STATIC_FN(&sys_prx_load_module, &callContext->handler)(callContext->args[0], callContext->args[1], callContext->args[2]);
                }
            }
        }
    }

    if (callContext->index == SYS_DBG_WRITE_PROCESS_MEMORY)
    {
        process_write_memory(proc, callContext->args[1], callContext->args[3], callContext->args[2], 1);
    }

    if (callContext->index == SYS_DBG_READ_PROCESS_MEMORY)
    {
        // TODO...
    }*/
}

/*void PpuThreadMsgInterruptExceptionOriginal(...)
{
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
    __asm("nop");
}

extern "C" void PpuThreadMsgInterruptExceptionHook(uint64_t thread_obj, uint64_t tb_value)
{
    DEBUG_PRINT("PpuThreadMsgInterruptExceptionHook\n");
    DEBUG_PRINT("thread_obj: 0x%llX\n", thread_obj);
    DEBUG_PRINT("tb_value: 0x%llX\n", tb_value);

    //PpuThreadMsgInterruptExceptionOriginal(thread_obj, tb_value); // HookFunctionStart
    lv2::ppu_thread_msg_interrupt_exception(thread_obj, tb_value);
}*/

/*LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_2(void, PpuThreadMsgInterruptExceptionHook, (uint64_t thread_obj, uint64_t tb_value))
{
    DEBUG_PRINT("PpuThreadMsgInterruptExceptionHook\n");
    DEBUG_PRINT("thread_obj: 0x%llX\n", thread_obj);
    DEBUG_PRINT("tb_value: 0x%llX\n", tb_value);



}*/

void InstallHooks()
{
    SpinLockInit(&SystemCallLock);
    PPCWriteBranchRelative(g_LibLV2.systemCallDispatchBranch, ((OPD_t*)SystemCallHookPrepareDispatch)->Function, true);



    //PPCWriteBranchRelative((uint32_t*)g_LibLV2.ppu_thread_msg_interrupt_exception_opd->Function, ((OPD_t*)PpuThreadMsgInterruptExceptionHookPrepare)->Function, false);
    //HookFunctionStart(g_LibLV2.ppu_thread_msg_interrupt_exception_opd->Function, ((OPD_t*)PpuThreadMsgInterruptExceptionHookPrepare)->Function, ((OPD_t*)PpuThreadMsgInterruptExceptionOriginal)->Function);
    //PPCWriteBranchRelative((uint32_t*)g_LibLV2.ppuThreadMsgInterruptExceptionBranch, ((OPD_t*)PpuThreadMsgInterruptExceptionHookPrepare)->Function, true);
    //hook_function_with_postcall(g_LibLV2.ppu_thread_msg_interrupt_exception_opd->Function, PpuThreadMsgInterruptExceptionHook, 2);
    //hook_function_on_precall_success(g_LibLV2.ppu_thread_msg_interrupt_exception_opd->Function, (void*)((OPD_t*)PpuThreadMsgInterruptExceptionHook)->Function, 2);
}
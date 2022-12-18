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

int sys_dbg_read_process_memory(lv2::sys_pid_t pid, void* destination, size_t size, void* source);
int dumpCounter = 0;
//extern "C" void PpuThreadMsgInterruptExceptionHookPrepare();


volatile int SystemCallLock;
extern "C" void SystemCallHookPrepareDispatch();
extern "C" void SystemCallHookProcess(SystemCallContext* callContext)
{
    if (callContext->index == SYS_DBG_WRITE_PROCESS_MEMORY) // TODO: add check for CEX consoles only
    {
        lv2::process* process = lv2::get_current_process();
        if (process != nullptr)
        {
            if (process->imageName != nullptr)
            {
                if (NonCryptoHashFNV1A64(process->imageName) == NonCryptoHashFNV1A64("/app_home/EBOOT.BIN")
                    || NonCryptoHashFNV1A64(process->imageName) == NonCryptoHashFNV1A64("/dev_bdvd/PS3_GAME/USRDIR/EBOOT.BIN")
                    || strstr(process->imageName, "PS3_GAME/USRDIR/")
                    || strstr(process->imageName, "hdd0/game/"))
                {
                    DEBUG_PRINT("SYS_DBG_WRITE_PROCESS_MEMORY\n");

                    uint32_t      procId = callContext->get_arg<uint32_t>(0);
                    lv2::process* currentProcess = lv2::get_process_by_pid(procId);

                    if (currentProcess)
                    {
                        DEBUG_PRINT("addr 0x%llx | size 0x%llx\n", callContext->get_arg<uint64_t>(1), callContext->get_arg<uint64_t>(2));
                        lv2::process_write_memory(currentProcess, callContext->get_arg<void*>(1), callContext->get_arg<void*>(3), callContext->get_arg<uint64_t>(2), 1);
                    }

                    //void** processList = (void**)(*(uint64_t*)(g_LibLV2.kernelTOC + 0x20E0));
                    //lv2::id_table_unreserve_id(*processList, handle);
                }
            }
        }
    }

    if (callContext->index == SYS_DBG_READ_PROCESS_MEMORY)
    {
        //DEBUG_PRINT("SYS_DBG_READ_PROCESS_MEMORY\n");

        // call the original method #1
        //STATIC_FN( &sys_dbg_read_process_memory, &callContext->handler )( callContext->args[0], (void*)callContext->args[1], callContext->args[2], (void*)callContext->args[3] );

        // call the original method #2
        //auto Error = sys_dbg_read_process_memory(callContext->args[0], callContext->args[1], callContext->args[2], callContext->args[3]);

        //DEBUG_PRINT("Error: 0x%llX\n", Error);

        // hook for HEN/CEX. We might need kmalloc and copy_from_user
        /*uint32_t      procId = callContext->args[0];
        lv2::process* process = lv2::get_process_by_pid(procId);

        if (process)
        {
            lv2::process_read_memory(process, (void*)callContext->args[1], (void*)callContext->args[2], callContext->args[3]);
        }*/
    }
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
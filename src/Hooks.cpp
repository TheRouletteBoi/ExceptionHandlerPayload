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

//extern "C" void PpuThreadMsgInterruptExceptionHookPrepare();

volatile int SystemCallLock;
extern "C" void SystemCallHookPrepareDispatch();
extern "C" void SystemCallHookProcess(SystemCallContext* syscall)
{
    if (syscall->index == SYS_DBG_WRITE_PROCESS_MEMORY) // TODO: add check for CEX consoles only
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


                    // Method #1 use syscall 8
                    auto ps3mapi_set_process_memory = SYSCALL_F(int(*)( int syscall8OpCode, int ps3mapiOpdCode, lv2::sys_pid_t pid, void* destination, const void* source, size_t size), 8 );

                    DEBUG_PRINT("ps3mapi_set_process_memory opd 0x%llx\n", ps3mapi_set_process_memory);

                    auto pid = syscall->get_arg<lv2::sys_pid_t>(0);
                    auto destination = syscall->get_arg<void*>(1);
                    auto size = syscall->get_arg<size_t>(2);
                    auto source = syscall->get_arg<const void*>(3);

                    DEBUG_PRINT("pid 0x%llx\n", pid);
                    DEBUG_PRINT("destination 0x%llx\n", destination);
                    DEBUG_PRINT("size 0x%llx\n", size);
                    DEBUG_PRINT("source 0x%llx\n", source);

                    DEBUG_PRINT("ps3mapi_set_process_memory\n");
                    auto Error = ps3mapi_set_process_memory(0x7777, 0x0032, pid, destination, source, size);
                    DEBUG_PRINT("Error 0x%X\n", Error);
                    

                    // method #2 recreate sys_dbg_write_process_memory
                    /*auto applicationPid = syscall->get_arg<lv2::sys_pid_t>(0);
                    if (process->processID == applicationPid)
                    {
                        DEBUG_PRINT("addr 0x%llx | size 0x%llx\n", syscall->get_arg<uint64_t>(1), syscall->get_arg<uint64_t>(2));
                        lv2::process_write_memory(applicationProcess, syscall->get_arg<void*>(1), syscall->get_arg<void*>(3), syscall->get_arg<uint64_t>(2), 1);
                    }*/

                    //lv2::unreserve_process_handle(processHandle);
                }
            }
        }
    }

    if (syscall->index == SYS_DBG_READ_PROCESS_MEMORY)
    {
        DEBUG_PRINT("SYS_DBG_READ_PROCESS_MEMORY\n");

        auto ps3mapi_get_process_memory = SYSCALL_F(int(*)( int syscall8OpCode, int ps3mapiOpdCode, lv2::sys_pid_t pid, void* destination, void* source, size_t size), 8 );

        auto pid = syscall->get_arg<lv2::sys_pid_t>(0);
        auto destination = syscall->get_arg<void*>(1);
        auto size = syscall->get_arg<size_t>(2);
        auto source = syscall->get_arg<void*>(3);

        DEBUG_PRINT("ps3mapi_get_process_memory\n");
        auto Error = ps3mapi_get_process_memory(0x7777, 0x0031, pid, destination, source, size);
        DEBUG_PRINT("Error 0x%X\n", Error);




        /*auto applicationPid = syscall->get_arg<lv2::sys_pid_t>(0);
        if (process->processID == applicationPid)
        {
            lv2::process_read_memory(applicationProcess, syscall->get_arg<void*>(1), syscall->get_arg<void*>(2), syscall->get_arg<uint64_t>(3));
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
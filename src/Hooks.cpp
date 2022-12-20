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

static INLINE void *get_secure_user_ptr(void *ptr)
{
	return (void *)(((uint64_t)ptr)&0xFFFFFFFF);
}

//extern "C" void PpuThreadMsgInterruptExceptionHookPrepare();

volatile int SystemCallLock;
extern "C" void SystemCallHookPrepareDispatch();
extern "C" void SystemCallHookProcess(SystemCallContext* syscall)
{
    if (syscall->index == _SYS_PRX_LOAD_MODULE)
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
                    DEBUG_PRINT("_SYS_PRX_LOAD_MODULE\n");

                    auto modulePath = syscall->GetArg<const char*>(0);
                    auto flags = syscall->GetArg<lv2::sys_prx_flags_t>(1);
                    auto pOpt = syscall->GetArg<lv2::sys_prx_module_info_option_t*>(2);

                    DEBUG_PRINT("modulePath %s\n", modulePath);
                    DEBUG_PRINT("flags 0x%llx\n", flags);
                    DEBUG_PRINT("pOpt 0x%llx\n", pOpt);

                    // load sprx into game without custom eboot. All games load libsysmodule.sprx
                    if (NonCryptoHashFNV1A64(modulePath) == NonCryptoHashFNV1A64("/dev_flash/sys/external/libsysmodule.sprx"))
                    {
                        //auto sys_prx_load_module = SYSCALL_F(int(*)(const char* path, lv2::sys_prx_flags_t flags, lv2::sys_prx_module_info_option_t* pOpt), _SYS_PRX_LOAD_MODULE);
                        //sys_prx_load_module("/dev_hdd0/plugins/mods/gtav.sprx", 0, NULL);
                    }

                    // override sprx path when loaded by custom eboot
                    if (NonCryptoHashFNV1A64(modulePath) == NonCryptoHashFNV1A64("/dev_hdd0/tmp/gtas.sprx"))
                    {
                        //modulePath = "/dev_hdd0/plugins/mods/gtas.sprx";
                    }
                }
            }
        }
    }

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

                    auto pid = syscall->GetArg<lv2::sys_pid_t>(0);
                    auto destination = syscall->GetArg<void*>(1);
                    auto size = syscall->GetArg<uint64_t>(2);
                    auto source = syscall->GetArg<void*>(3);

                    DEBUG_PRINT("pid 0x%llx\n", pid);
                    DEBUG_PRINT("destination 0x%llx\n", destination);
                    DEBUG_PRINT("size 0x%llx\n", size);
                    DEBUG_PRINT("source 0x%llx\n", source);


                    // Method #1 use syscall 8
                    /*auto ps3mapi_set_process_memory = SYSCALL_F(int(*)( int syscall8OpCode, int ps3mapiOpdCode, lv2::sys_pid_t pid, void* destination, void* source, uint64_t size), 8 );
                    DEBUG_PRINT("ps3mapi_set_process_memory\n");
                    auto Error = ps3mapi_set_process_memory(0x7777, 0x0032, pid, destination, source, size);
                    DEBUG_PRINT("Error 0x%X\n", Error);*/
                    

                    // method #2 recreate sys_dbg_write_process_memory
                    if (process->processID == pid)
                    {
                        lv2::process_write_memory(process, destination, source, size, 1);
                    }

                    //lv2::unreserve_process_handle(processHandle);
                }
            }
        }
    }

    if (syscall->index == SYS_DBG_READ_PROCESS_MEMORY)
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
                    DEBUG_PRINT("SYS_DBG_READ_PROCESS_MEMORY\n");

                    auto pid = syscall->GetArg<lv2::sys_pid_t>(0);
                    auto destination = syscall->GetArg<void*>(1);
                    auto source = syscall->GetArg<void*>(2);
                    auto size = syscall->GetArg<uint64_t>(3);

                    DEBUG_PRINT("pid 0x%llx\n", pid);
                    DEBUG_PRINT("destination 0x%llx\n", destination);
                    DEBUG_PRINT("source 0x%llx\n", source);
                    DEBUG_PRINT("size 0x%llx\n", size);


                    // Method #1 use syscall 8
                    /*auto ps3mapi_get_process_memory = SYSCALL_F(int(*)( int syscall8OpCode, int ps3mapiOpdCode, lv2::sys_pid_t pid, void* destination, void* source, uint64_t size), 8 );
                    DEBUG_PRINT("ps3mapi_get_process_memory\n");
                    auto Error = ps3mapi_get_process_memory(0x7777, 0x0031, pid, destination, source, size);
                    DEBUG_PRINT("Error 0x%X\n", Error);*/


                    // Method #2 recreate sys_dbg_write_process_memory
                    if (process->processID == pid)
                    {
                        lv2::process_read_memory(process, destination, source, size);
                    }
                }
            }
        }
    }
}

/*
const char* g_OverrideSprxPathFrom  = NULL;
const char* g_OverrideSprxPathTo    = NULL;

const char* g_OverrideExecutablePath    = NULL;
bool        g_OverrideExecutableToggle  = false;

// OR

struct EbootToLoad
{
    char moduleToLoad[512];
    char ebootToLoad[512];
};

void syscall_OverrideSprxPath(const char* from const char* to)
{
    g_OverrideSprxPathFrom = from;
    g_OverrideSprxPathTo = to;
}

// **CAVEATS**
// 1: NOT viable since we have to check for game region.
// 2: I have no way to detecting the game the user is going to start up??? unless you give them a input dialog of the games they have and they will select.
// 3: some games eboots require patches in order for the game to load
// 4: if loaded with custom eboot path and sprx path in eboot is for example /dev_hdd0/tmp/name.sprx then it will load that tmp path which we don't want. so we will need to strcmp in LOAD_MODULE syscall
void syscall_OverrideExecutablePath(const char* path, bool toggle)
{
    g_OverrideExecutablePath = path;
    g_OverrideExecutableToggle = toogle;
}

int PpuLoaderLoadProgramHook(lv2::process* process, int fd, const char* path, int r6, uint64_t r7, uint64_t r8, uint64_t r9, uint64_t r10, uint64_t sp_70)
{
    if (g_OverrideExecutableToggle && g_OverrideExecutablePath)
    {
        // check that we are a game
        if (strstr(path, "PS3_GAME/USRDIR/") || strstr(path, "hdd0/game/"))
        {
            if (strstr(path, g_OverrideExecutablePath))
            {
                path = g_OverrideExecutablePath;
                g_OverrideExecutableToggle = false;
            }
        }
    }
}*/

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
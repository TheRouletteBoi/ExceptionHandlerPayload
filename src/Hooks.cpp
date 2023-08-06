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

lv2::sys_prx_id_t sys_prx_load_module(
    const char* path,
    lv2::sys_prx_flags_t flags, 
    lv2::sys_prx_load_module_option_t* pOpt
);

bool IsGameProcess(lv2::process* process)
{
    if (NonCryptoHashFNV1A64(process->imageName) == NonCryptoHashFNV1A64("/app_home/EBOOT.BIN")
        || NonCryptoHashFNV1A64(process->imageName) == NonCryptoHashFNV1A64("/dev_bdvd/PS3_GAME/USRDIR/EBOOT.BIN")
        || strstr(process->imageName, "PS3_GAME/USRDIR/")
        || strstr(process->imageName, "hdd0/game/"))
        return true;
        
    return false;
}

void SysPrxLoadModuleHook(SystemCallContext* syscall)
{
    lv2::process* process = lv2::get_current_process();
    
    if (process == nullptr)
        return;
        
    if (process->imageName == nullptr)
        return;

    if (IsGameProcess(process))
    {
        DEBUG_PRINT("_SYS_PRX_LOAD_MODULE\n");

        const auto modulePath   = syscall->GetArg<const char*>(0);
        const auto flags        = syscall->GetArg<lv2::sys_prx_flags_t>(1);
        const auto pOpt         = syscall->GetArg<lv2::sys_prx_module_info_option_t*>(2);

        DEBUG_PRINT("modulePath %s\n", modulePath);
        DEBUG_PRINT("flags 0x%llx\n", flags);
        DEBUG_PRINT("pOpt 0x%llx\n", pOpt);




        auto memoryManager = AllocateUserPage(process, 0x2000);

        if (memoryManager.kernelPage == NULL)
            return;

        const auto sprxModuleInfoOpt = memoryManager.Get<lv2::sys_prx_module_info_option_t>(1);
        const auto sprxModuleInfo    = memoryManager.Get<lv2::sys_prx_module_info_t>(1);
        const auto sprxName          = memoryManager.Get<char>(256);

        sprxModuleInfoOpt->size         = 0x10;
        sprxModuleInfoOpt->moduleInfo32 = sprxModuleInfo.UserAs<uint32_t>();

        sprxModuleInfo->size         = sizeof(lv2::sys_prx_module_info_t);
        sprxModuleInfo->fileName     = sprxName.UserAs<uint32_t>();
        sprxModuleInfo->fileNameSize = 256;

        auto sys_prx_get_module_info = SYSCALL_F(int(*)(lv2::sys_prx_id_t id, lv2::sys_prx_flags_t flags, lv2::sys_prx_module_info_option_t *pInfo), _SYS_PRX_GET_MODULE_INFO);

        // TODO(Roulette): replace syscall->args[ xx ] with syscall->GetArg<type>(xx);
        auto error = sys_prx_get_module_info(syscall->args[0], 0, sprxModuleInfoOpt.User());

        DEBUG_PRINT("sys_prx_load_module: %x %s\n", error, sprxModuleInfo->name);

        STATIC_FN(&sys_prx_load_module, &syscall->handler)((const char*)syscall->args[0], syscall->args[1], (lv2::sys_prx_load_module_option_t*)syscall->args[2]);
        
        
        /*** 
        * every single game uses libsysmodule.sprx so we will use it to inject our sprx into the game without custom EBOOT
        * this is useful to load an sprx without the need to create a eboot with sprx loader. this is only for sprx's with 
        * large sprx which will not work when using sys_prx_load_module() when fully in game or while using a sprx tool injector
        * NOTE(Roulette): will require a custom syscall??? for us to get new_path from user
        * EG: custom_syscall_mimic_eboot_sprx_loader("/dev_hdd0/tmp/gtas.sprx");
        */
        if (NonCryptoHashFNV1A64(sprxModuleInfo->name) == NonCryptoHashFNV1A64("libsysmodule.sprx"))
        {
#if 0
            // TODO(Roulette): should I replace the string or the pointer to the string??

            // get memory address of arg
            // replace string in bytes
            auto modulePathAddress = &syscall->args[0];
            memcpy(modulePathAddress, newPath, sizeof(newPath));
            PPCFlushInstructionCache(modulePathAddress);


            /* OR */


            // replace pointer of string
            modulePath = newPath;
#endif
        }

        /***
         * if the eboot loads a sprx path this function will override sprx path  
         * NOTE(Roulette): will require a custom syscall??? for us to get old_path and new_path from user
         * EG: custom_syscall_override_eboot_sprx_loader(old_path, new_path);
         * @param old_path would be sprx path in modded eboot
        */
#if 0
        if (NonCryptoHashFNV1A64(modulePath) == NonCryptoHashFNV1A64(oldPath))
        {
            modulePath = newPath;
        }
#endif

        FreeUserModePages(process, memoryManager);
    }
}

// TODO(Roulette): add check for CEX consoles only
void SysDbgWriteProcessMemoryHook(SystemCallContext* syscall)
{
    lv2::process* process = lv2::get_current_process();
    
    if (process == nullptr)
        return;
        
    if (process->imageName == nullptr)
        return;

    if (IsGameProcess(process))
    {
        DEBUG_PRINT("SYS_DBG_WRITE_PROCESS_MEMORY\n");

        const auto pid          = syscall->GetArg<lv2::sys_pid_t>(0);
        const auto destination  = syscall->GetArg<void*>(1);
        const auto size         = syscall->GetArg<uint64_t>(2);
        const auto source       = syscall->GetArg<void*>(3);

        DEBUG_PRINT("pid 0x%llx\n", pid);
        DEBUG_PRINT("destination 0x%llx\n", destination);
        DEBUG_PRINT("size 0x%llx\n", size);
        DEBUG_PRINT("source 0x%llx\n", source);


#if 1
        // method #1 redirect to syscall 8
        auto syscall8 = SYSCALL_F(int(*)(uint64_t function, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5, uint64_t param6, uint64_t param7), 8);
        DEBUG_PRINT("ps3mapi_set_process_memory\n");
        auto error = syscall8(0x7777, 0x0032, syscall->args[0], syscall->args[1], syscall->args[3], syscall->args[2], 0, 0);
        DEBUG_PRINT("Error 0x%X\n", error);
#else
        // method #2 recreate sys_dbg_write_process_memory
        if (process->processID != pid)
            return;

        lv2::extend_kstack(0);
        if (!size || !source)
            return 0x80010409;

        lv2::process* lv2Process;
        void*    processHandle  = NULL;
        uint32_t error          = lv2::get_process_object_by_id(pid, &lv2Process, &processHandle);
        if (!error)
        {
            lv2Buffer = lv2::kmalloc(size, 0xD, 0);
            if (lv2Buffer)
            {
                error = lv2::copy_from_user(source, lv2Buffer, size);
                if (!error)
                {
                    error = lv2::process_write_memory(lv2Process, destination, lv2Buffer, size, 1);
                    if (error)
                        error = 0x8001000D;
                }
                lv2::kfree(lv2Buffer, 0xD);
                lv2::unreserve_process_handle(processList, processHandle);
            }
            else
            {
                error = 0x80010408;
                lv2::unreserve_process_handle(processList, processHandle);
            }
        }
        return error;
#endif
    }
}

void SysDbgReadProcessMemoryHook(SystemCallContext* syscall)
{
    lv2::process* process = lv2::get_current_process();
    
    if (process == nullptr)
        return;
        
    if (process->imageName == nullptr)
        return;

    if (IsGameProcess(process))
    {
        DEBUG_PRINT("SYS_DBG_READ_PROCESS_MEMORY\n");

        const auto pid          = syscall->GetArg<lv2::sys_pid_t>(0);
        const auto destination  = syscall->GetArg<void*>(1);
        const auto source       = syscall->GetArg<void*>(2);
        const auto size         = syscall->GetArg<uint64_t>(3);

        DEBUG_PRINT("pid 0x%llx\n", pid);
        DEBUG_PRINT("destination 0x%llx\n", destination);
        DEBUG_PRINT("source 0x%llx\n", source);
        DEBUG_PRINT("size 0x%llx\n", size);


#if 1
        // method #1 redirect to syscall 8
        auto syscall8 = SYSCALL_F(int(*)(uint64_t function, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5, uint64_t param6, uint64_t param7), 8);
        DEBUG_PRINT("ps3mapi_get_process_memory\n");
        auto error = syscall8(0x7777, 0x0031, syscall->args[0], syscall->args[1], syscall->args[2], syscall->args[3], 0, 0);

        // TODO(roulette): need to return error
#else
        // Method #2 recreate sys_dbg_read_process_memory
        if (process->processID != pid)
            return;

        extend_kstack(0);
        uint32_t error = 0x80010409;

        if (!size || !source)
            return error;

        lv2::process* lv2Process;
        void* processHandle     = NULL;
        uint32_t error          = lv2::get_process_object_by_id(pid, &lv2Process, &processHandle);
        if (!error)
        {
            lv2Buffer = lv2::kmalloc(size, 0xD, 0);
            error = 0x80010408;
            if (lv2Buffer)
            {
                error = 0x8001000D;
                if (!lv2::process_read_memory(lv2Process, destination, lv2Buffer, size, 1))
                {
                    lv2::unreserve_process_handle(processList, processHandle);
                    canCopyToUser = lv2::copy_to_user(lv2Buffer, source, size);
                    lv2::kfree(lv2Buffer, 0xD);
                    return canCopyToUser;
                }

                lv2::kfree(lv2Buffer, 0xD);
            }

            lv2::unreserve_process_handle(processList, processHandle);
            return error;
        }
        return error;
#endif
    }
}

volatile int SystemCallLock;
extern "C" void SystemCallHookPrepareDispatch();
extern "C" void SystemCallHookProcess(SystemCallContext* syscall)
{
    if (syscall->index == _SYS_PRX_LOAD_MODULE)
        SysPrxLoadModuleHook(syscall);

    if (syscall->index == SYS_DBG_WRITE_PROCESS_MEMORY)
        SysDbgWriteProcessMemoryHook(syscall);

#if 0
    if (syscall->index == SYS_DBG_READ_PROCESS_MEMORY)
        SysDbgReadProcessMemoryHook(syscall);
#endif
}


#if 0

char g_OverrideSprxPathFrom[1000];
char g_OverrideSprxPathTo[1000];

char g_OverrideExecutablePath[1000];
bool g_OverrideExecutableToggle = false;

struct EbootToLoad
{
    char moduleToLoad[1000];
    char ebootToLoad[1000];
};

void syscall_override_eboot_sprx_loader(const char* from const char* to)
{
    strncpy(g_OverrideSprxPathFrom, from);
    strncpy(g_OverrideSprxPathTo, to);      
}

/*  **CAVEATS**
*   1: NOT viable since we have to check for game region.
*   2: I have no way to detecting the game the user is going to start up??? unless you give them a input dialog of the games they have and they will select.
*   3: some games eboots require patches in order for the game to load
*   4: if loaded with custom eboot path and sprx path in eboot is for example /dev_hdd0/tmp/name.sprx then it will load that tmp path which we don't want. so we will need to strcmp in LOAD_MODULE syscall
*/
void syscall_override_eboot_path(const char* path, bool toggle)
{
    strncpy(g_OverrideExecutablePath, path);
    g_OverrideExecutableToggle = toogle;
}

int PpuLoaderLoadProgramHook(lv2::process* process, int fd, const char* path, int r6, uint64_t r7, uint64_t r8, uint64_t r9, uint64_t r10, uint64_t sp_70)
{
    if (g_OverrideExecutableToggle)
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
}

#endif

#if 0
extern "C" void PpuThreadMsgInterruptExceptionHookPrepare();
void PpuThreadMsgInterruptExceptionOriginal(...)
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
}
#endif

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
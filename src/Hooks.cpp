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
#include "HookOpCode.h"

/*
* NOTE
* if you see any hardcoded memory addresses they are for kernel/LV2 4.84 DEX
*/

lv2::sys_prx_id_t sys_prx_load_module_fn(
    const char* path,
    lv2::sys_prx_flags_t flags, 
    lv2::sys_prx_load_module_option_t* pOpt
);

int sys_dbg_write_process_memory_fn(
    lv2::sys_pid_t pid,
    void* destination, 
    uint64_t size,
    void* source
);

void NopFn()
{

}

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

        auto moduleId = STATIC_FN(&sys_prx_load_module_fn, &syscall->handler)(syscall->GetArg<const char*>(0), syscall->GetArg<lv2::sys_prx_flags_t>(1), syscall->GetArg<lv2::sys_prx_load_module_option_t*>(2));
        DEBUG_PRINT("moduleId: %x\n", moduleId);

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

        auto error = sys_prx_get_module_info(moduleId, 0, sprxModuleInfoOpt.User());

        DEBUG_PRINT("sys_prx_get_module_info: %x %s\n", error, sprxModuleInfo->name);

        /*** 
        * every single game uses libsysmodule.sprx alias cellSysmodule_Library so we will use it to inject our sprx into the game without custom EBOOT
        * this is useful to load a sprx without the need to create a eboot with sprx loader built in. This is more viable for sprx's
        * that exceed memory limits and sprs's that aren't able to load using sys_prx_load_module(). Especially when fully in game or while using a sprx injector
        * NOTE(Roulette): will require a custom syscall??? for us to get new_path from user
        * EG: custom_syscall_mimic_eboot_sprx_loader("/dev_hdd0/tmp/gtas.sprx");
        */
        if (NonCryptoHashFNV1A64(sprxModuleInfo->name) == NonCryptoHashFNV1A64("cellSysmodule_Library"))
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
        //lv2::extend_kstack(0);

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
        // delete redirect original function to our NopFn function which does nothing
        auto error = STATIC_FN(&sys_dbg_write_process_memory_fn, &syscall->handler)(pid, destination, size, source);
        DEBUG_PRINT("sys_dbg_write_process_memory_fn returned = 0x%llX\n", error);
        syscall->handler.Function = ((OPD_t*)NopFn)->Function;

        // method #1 redirect to syscall 8
        auto syscall8 = SYSCALL_F(int(*)(uint64_t function, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5, uint64_t param6, uint64_t param7), 8);
        DEBUG_PRINT("calling syscall8 with parameters [arg1] = 0x%llX [arg2] = 0x%llX [arg3] = 0x%llX [arg4] = 0x%llX [arg5] = 0x%llX [arg6] = 0x%llX [arg7] = 0x%llX [arg8] = 0x%llX\n", 0x7777, 0x0032, pid, destination, source, size, 0, 0);
        
        // BUG(roulette): crashes when calling syscall8
        error = syscall8((uint64_t)0x7777, (uint64_t)0x0032, (uint64_t)pid, (uint64_t)destination, (uint64_t)source, (uint64_t)size, (uint64_t)0, (uint64_t)0);
        DEBUG_PRINT("syscall8 returned = 0x%llX\n", error);
#endif

#if 0
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

#if 0
        // method #3 redirect parameters | Update: doesn't work because we write back to SystemCallContext* which not the actual parameters/registers
        syscall->index = 8;
        syscall->SetArg(0, 0x7777);
        syscall->SetArg(1, 0x0032);
        syscall->SetArg(2, syscall->GetArg<lv2::sys_pid_t>(0));
        syscall->SetArg(3, syscall->GetArg<void*>(1));
        syscall->SetArg(4, syscall->GetArg<void*>(3));
        syscall->SetArg(5, syscall->GetArg<uint64_t>(2));
        syscall->SetArg(6, 0);
        syscall->SetArg(7, 0);
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
        DEBUG_PRINT("calling ps3mapi_get_process_memory with parmeters [arg1] = 0x%016llX [arg2] = 0x%016llX [arg3] = 0x%016llX [arg4] = 0x%016llX [arg5] = 0x%016llX [arg6] = 0x%016llX\n", syscall->args[0], syscall->args[1], syscall->args[2], syscall->args[3], 0, 0);
        auto error = syscall8(0x7777, 0x0031, syscall->args[0], syscall->args[1], syscall->args[2], syscall->args[3], 0, 0);
        DEBUG_PRINT("ps3mapi_get_process_memory returned = 0x%016llX\n", error);

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
#if 0
    if (syscall->index == _SYS_PRX_LOAD_MODULE)
        SysPrxLoadModuleHook(syscall);
#endif

#if 1
    if (syscall->index == SYS_DBG_WRITE_PROCESS_MEMORY)
        SysDbgWriteProcessMemoryHook(syscall);
#endif

#if 0
    if (syscall->index == SYS_DBG_READ_PROCESS_MEMORY)
        SysDbgReadProcessMemoryHook(syscall);
#endif
}

#if OVERRIDE_EXECUTABLE_PATH
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
#endif


// TODO(Roulette): move function to process header
static inline uint32_t PpuLoaderLoadProgram(uint64_t process, uint64_t fd, uint64_t path, uint64_t r6, uint64_t r7, uint64_t r8, uint64_t r9, uint64_t r10, uint64_t stackPointer_0x70)
{
    return STATIC_FN(&PpuLoaderLoadProgram, g_LibLV2.ppuLoaderLoadProgram_opd)(process, fd, path, r6, r7, r8, r9, r10, stackPointer_0x70);
}

void PpuLoaderLoadProgramHook(HookOpCode::HookContext* registers)
{
    /*
    ROM:8000000000272584                 lwz       r6, 0x240+arg_1C(r1)
    ROM:8000000000272588                 extsw     r4, r27       
    ROM:800000000027258C                 mr        r5, r28       
    ROM:8000000000272590                 extsw     r6, r6        
    ROM:8000000000272594                 addi      r29, r31, 0x2E4
    ROM:8000000000272598                 mr        r3, r31       
    ROM:800000000027259C                 addi      r7, r1, 0x240+var_1A0
    ROM:80000000002725A0                 addi      r8, r31, 0x388 
    ROM:80000000002725A4                 addi      r9, r31, 0x330 
    ROM:80000000002725A8                 addi      r10, r31, 0x2C0 
    ROM:80000000002725AC                 std       r29, 0x240+var_1D0(r1)
    ROM:80000000002725B0                 bl        ppu_loader_load_program            <---- We come from here.
    ROM:80000000002725B4                 cmpwi     cr7, r3, 0
    ROM:80000000002725B8                 mr        r30, r3
    ROM:80000000002725BC                 bne       cr7, loc_80000000002728B4
    */


    // Original instruction at 0x80000000002725B0 is 'bl ppu_loader_load_program'
    // BUG(Roulette): crashes 'stack overflow' last parameter may be wrong
    uint32_t error = PpuLoaderLoadProgram(registers->r3, registers->r4, registers->r5, registers->r6, registers->r7, registers->r8, registers->r9, registers->r10, registers->r29);

    DEBUG_PRINT("PpuLoaderLoadProgramHook\n");
    DEBUG_PRINT("error 0x%llX\n", error);

    const auto process  = registers->GetGpr<lv2::process*>(3);
    const auto fd       = registers->GetGpr<int>(4);
    const auto path     = registers->GetGpr<const char*>(5);
    const auto r6       = registers->GetGpr<int>(6);
    const auto r7       = registers->GetGpr<uint64_t>(7);
    const auto r8       = registers->GetGpr<uint64_t>(8);
    const auto r9       = registers->GetGpr<uint64_t>(9);
    const auto r10      = registers->GetGpr<uint64_t>(10);
    const auto stackPointer_0x70       = registers->GetGpr<uint64_t>(29);

    DEBUG_PRINT("process: 0x%016llX\n", process);
    DEBUG_PRINT("fd: 0x%016llx\n", fd);
    DEBUG_PRINT("path: %s\n", path);
    DEBUG_PRINT("r6: 0x%016llx\n", r6);
    DEBUG_PRINT("r7: 0x%016llx\n", r7);
    DEBUG_PRINT("r8: 0x%016llx\n", r8);
    DEBUG_PRINT("r9: 0x%016llx\n", r9);
    DEBUG_PRINT("r10: 0x%016llx\n", r10);
    DEBUG_PRINT("stackPointer_0x70: 0x%016llx\n", stackPointer_0x70);

    // return our result??
    registers->r3 = error;

#if OVERRIDE_EXECUTABLE_PATH
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
#endif
}

void PpuThreadMsgInterruptExceptionHook(HookOpCode::HookContext* registers)
{
    // Original instruction at 0x800000000026C348 is 'std       r25, 0xB0+var_38(r1)'
    *(uint64_t*)(registers->r1 + 0x78) = registers->r25;

    DEBUG_PRINT("PpuThreadMsgInterruptExceptionHook\n");

    const auto thread_obj   = registers->GetGpr<uint64_t>(3);
    const auto tb_value     = registers->GetGpr<uint64_t>(4);

    DEBUG_PRINT("thread_obj: 0x%016llX\n", thread_obj);
    DEBUG_PRINT("tb_value: 0x%016llx\n", tb_value);

    DEBUG_PRINT("r3: 0x%016llX\n", registers->r3);
    DEBUG_PRINT("r4: 0x%016llx\n", registers->r4);
}

void createProcessCommonHook(HookOpCode::HookContext* registers)
{
    // Original instruction at 0x8000000000272470 is 'std       r18, 0x240+var_100(r1)'
    *(uint64_t*)(registers->r1 + 0x140) = registers->r18;

    DEBUG_PRINT("createProcessCommonHook\n");
    const auto path   = registers->GetGpr<uint64_t>(6);

    DEBUG_PRINT("path: %s\n", path);
}


#if 0
OPD_t* sysDbgWriteProcessMemoryOriginal = nullptr;
void DetourSyscall(uint32_t syscallIndex, OPD_t* fn, OPD_t** original)
{
    // switch fumction payload TOC with kernel TOC
    fn->TOC = g_LibLV2.kernelTOC;

    // get the original unmodified values
    *original = g_LibLV2.systemCallTable[syscallIndex];

    // overwrite syscall OPD_t
    g_LibLV2.systemCallTable[syscallIndex] = fn;
}

extern "C" void HookSyscallPrepareDispatch();
extern "C" int sysDbgWriteProcessMemoryHook2(lv2::sys_pid_t pid, void* destination, uint64_t size, void* source)
{
#if 0
    lv2::process* process = lv2::get_current_process();
    
    if (process == nullptr)
        return callOriginal;
        
    if (process->imageName == nullptr)
        return callOriginal;

    if (IsGameProcess(process))
    {
        //lv2::extend_kstack(0);

        DEBUG_PRINT("sysDbgWriteProcessMemoryHook2\n");

        DEBUG_PRINT("pid 0x%llx\n", pid);
        DEBUG_PRINT("destination 0x%llx\n", destination);
        DEBUG_PRINT("size 0x%llx\n", size);
        DEBUG_PRINT("source 0x%llx\n", source);

        auto syscall8 = SYSCALL_F(int(*)(uint64_t function, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5, uint64_t param6, uint64_t param7), 8);
        DEBUG_PRINT("calling syscall8 with parameters [arg1] = 0x%llX [arg2] = 0x%llX [arg3] = 0x%llX [arg4] = 0x%llX [arg5] = 0x%llX [arg6] = 0x%llX [arg7] = 0x%llX [arg8] = 0x%llX\n", 0x7777, 0x0032, pid, destination, source, size, 0, 0);
        
        // BUG(roulette): crashes when calling syscall8
        auto error = syscall8((uint64_t)0x7777, (uint64_t)0x0032, (uint64_t)pid, (uint64_t)destination, (uint64_t)source, (uint64_t)size, (uint64_t)0, (uint64_t)0);
        DEBUG_PRINT("syscall8 returned = 0x%llX\n", error);
    }
#endif

    DEBUG_PRINT("Calling original\n");
    int error = CallByOpd<int>(sysDbgWriteProcessMemoryOriginal, pid, destination, size, source);
    DEBUG_PRINT("returning = 0x%llX\n", error);
    return error;
}
#endif

void InstallHooks()
{
#if 0
    SpinLockInit(&SystemCallLock);
    PPCWriteBranchRelative(g_LibLV2.systemCallDispatchBranch, ((OPD_t*)SystemCallHookPrepareDispatch)->Function, true);
#endif

#if 1
    DEBUG_PRINT("HookOpCode::AttachDetour\n");
    HookOpCode::AttachDetour(g_LibLV2.ppuThreadMsgInterruptException3rdInstructionAddress, PpuThreadMsgInterruptExceptionHook);
    HookOpCode::AttachDetour(g_LibLV2.ppuLoaderLoadProgramBranch, PpuLoaderLoadProgramHook);
    //HookOpCode::AttachDetour(0x8000000000272470, createProcessCommonHook);
    //DetourSyscall(SYS_DBG_WRITE_PROCESS_MEMORY, (OPD_t*)HookSyscallPrepareDispatch, &sysDbgWriteProcessMemoryOriginal);
#endif
}
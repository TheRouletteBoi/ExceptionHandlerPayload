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
    PPCWriteBranchRelative(g_LibLV2.ppuThreadMsgInterruptException, ((OPD_t*)PpuThreadMsgInterruptExceptionHook)->Function, false);
}

int sys_prx_unload_module(lv2::sys_prx_id_t id, lv2::sys_prx_flags_t flags, lv2::sys_prx_unload_module_option_t* pOpt);

void Nop()
{

}

extern "C" void SystemCallHookProcess(SystemCallContext* callContext)
{
    if (callContext->index == _SYS_PRX_UNLOAD_MODULE)
    {
        lv2::process* process = lv2::get_current_process();

        if (process != NULL)
        {
            if (process->imageName != NULL)
            {
                // Check we are coming from the VSH.
                if (NonCryptoHashFNV1A64(process->imageName) == NonCryptoHashFNV1A64("/dev_flash/vsh/module/vsh.self"))
                {
                    UserMappedMemory memoryManager = AllocateUserPage(process, 0x2000);

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

                    auto sys_prx_get_module_info =  SYSCALL_F(int(*)(lv2::sys_prx_id_t id, lv2::sys_prx_flags_t flags, lv2::sys_prx_module_info_option_t *pInfo), _SYS_PRX_GET_MODULE_INFO);

                    auto error = sys_prx_get_module_info(callContext->args[0], 0, sprxModuleInfoOpt.User());

                    DEBUG_PRINT("sys_prx_unload_module: %x %s\n", error, sprxModuleInfo->name);


                    // Allow web browser to unload before triggering the exploit.
                    // This fixes crashes, most likely due to the sys_prx_unload_module calling extend_kstack.
                    STATIC_FN(&sys_prx_unload_module, &callContext->handler)(callContext->args[0], callContext->args[1], (lv2::sys_prx_unload_module_option_t*)callContext->args[2]);

                    callContext->handler.Function = ((OPD_t*)Nop)->Function;

                    // Run exploit on browser exit.
                    if (NonCryptoHashFNV1A64(sprxModuleInfo->name) == NonCryptoHashFNV1A64("webrender_plugin_module"))
                    {
                        DoExploit(callContext, process);
                    }

                    FreeUserModePages(process, memoryManager);

                    DEBUG_PRINT("Nice Poop\n");
                }
            }
        }
    }
}

void PpuThreadMsgInterruptExceptionHook()
{
    DEBUG_PRINT("PpuThreadMsgInterruptExceptionHook\n");
}
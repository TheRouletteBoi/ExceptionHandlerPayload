#include "LV2/LV2.h"
#include "LV2/Process.h"
#include "LV2/Memory.h"
#include "LV2/Network.h"
#include "LV2/Scheduler.h"
#include "LV2/Thread.h"

#include "UserMemory.h"

UserMappedMemory AllocateUserPage(lv2::process* process, size_t size, uint64_t exportFlags)
{
    void*    kernelPage = NULL;
    uint64_t userPage = NULL;

    auto error = lv2::page_allocate(process, size, 0x2F, lv2::page_size::PAGE_SIZE_4K, &kernelPage);

    if (error)
    {
        DEBUG_PRINT( "lv2::page_allocate error: %x\n", error);
        return UserMappedMemory(0, 0, 0);
    }

    DEBUG_PRINT("Allocated page at: 0x%llx\n", kernelPage);

    error = lv2::kmem_export_to_proc(process, kernelPage, exportFlags, &userPage);

    if (error)
    {
        DEBUG_PRINT("lv2::kmem_export_to_proc error: %x\n", error);
        return UserMappedMemory(0, 0, 0);
    }

    DEBUG_PRINT("Exported page to: 0x%llx\n", userPage);

    return UserMappedMemory(kernelPage, userPage, size);
}

void FreeUserModePages(lv2::process* process, UserMappedMemory& memoryManager)
{
    auto error = lv2::kmem_unexport_from_proc(process, memoryManager.userPage);

    DEBUG_PRINT("Unexporting page: 0x%llx\n", memoryManager.userPage);

    if (error)
    {
        DEBUG_PRINT("lv2::kmem_unexport_from_proc error: %x\n", error);
        return;
    }

    DEBUG_PRINT("Freeing page: 0x%llx\n", memoryManager.kernelPage);

    error = lv2::page_free(process, memoryManager.kernelPage, 0x2F);

    if (error)
    {
        DEBUG_PRINT("lv2::page_free error: %x\n", error);
    }

    DEBUG_PRINT("Page freed: 0x%llx\n", memoryManager.kernelPage);
}

void AllocateUserModePagesWithCallback(lv2::process* process, size_t size, void(*callback)(UserMappedMemory& memory))
{
    auto memoryManager = AllocateUserPage(process, size);

    callback(memoryManager);

    FreeUserModePages(process, memoryManager);
}
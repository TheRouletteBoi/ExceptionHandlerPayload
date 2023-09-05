#include "HookOpCode.h"
#include "PPC.h"

extern "C" void HookOpCodePrepareDispatch();

namespace HookOpCode
{
    HookEntry hookStack[MAX_HOOKS];
    size_t hookStackCounter;

    void Handler(HookContext* ctx)
    {
        if (ctx == nullptr)
            return;

        uintptr_t calledFrom = ctx->lr - 4;

        // Look through all our hooks to find the corresponding callback
        for (int i = 0; i < MAX_HOOKS; i++)
        {
            if (hookStack[i].address != calledFrom)
                continue;

            return hookStack[i].callback(ctx);
        }

        // Pause process if this function is not called from a hook we installed
        __builtin_trap();
    }

    void AttachDetour(uintptr_t address, HookCallback callback)
    {
        // If the address isn't valid don't go further
        if (address < 0x8000000000000000)
        {
            DEBUG_PRINT("ERROR: your hook address is not valid\n");
            return;
        }

        // Stop proceeding if we have reached our maximum amount of hooks
        if (hookStackCounter > MAX_HOOKS)
        {
            DEBUG_PRINT("ERROR: you have reached maximum amount of hooks. Change MAX_HOOKS limit\n");
            return;
        }

        // Setup the hook
        hookStack[hookStackCounter].address = address;
        hookStack[hookStackCounter].instruction = *(uint32_t*)address;
        hookStack[hookStackCounter].callback = callback;
        hookStackCounter++;

        // Branch address to HookOpCodePrepareDispatch()
        PPCWriteBranchRelative((uint32_t*)address, ((OPD_t*)HookOpCodePrepareDispatch)->Function, true);
    }
}